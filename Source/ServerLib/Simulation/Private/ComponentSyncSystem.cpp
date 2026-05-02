#include "ComponentSyncSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "Network.h"
#include "GraphicData.h"
#include "ReplicatedComponent.h"
#include "ClientSimData.h"
#include "Cylinder.h"
#include "Collision.h"
#include "SharedConfig.h"
#include "Log.h"
#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/map.hpp"
#include "boost/mp11/bind.hpp"
#include "tracy/Tracy.hpp"

namespace AM
{
namespace Server
{

/** Self-Init, Self-Update Observers. */
using SelfInitComponentTypes
    = boost::mp11::mp_append<EngineSelfInitComponentTypes,
                             ProjectSelfInitComponentTypes>;
using SelfUpdateComponentTypes
    = boost::mp11::mp_append<EngineSelfUpdateComponentTypes,
                             ProjectSelfUpdateComponentTypes>;
std::array<EnttObserver, boost::mp11::mp_size<SelfInitComponentTypes>::value>
    selfConstructObservers{};
std::array<EnttObserver, boost::mp11::mp_size<SelfUpdateComponentTypes>::value>
    selfUpdateObservers{};
std::array<EnttObserver, boost::mp11::mp_size<SelfInitComponentTypes>::value>
    selfDestroyObservers{};

/** InRange-Init, InRange-Update Observers. */
using InRangeInitComponentTypes
    = boost::mp11::mp_append<EngineInRangeInitComponentTypes,
                             ProjectInRangeInitComponentTypes>;
using InRangeUpdateComponentTypes
    = boost::mp11::mp_append<EngineInRangeUpdateComponentTypes,
                             ProjectInRangeUpdateComponentTypes>;
std::array<EnttObserver, boost::mp11::mp_size<InRangeInitComponentTypes>::value>
    inRangeConstructObservers{};
std::array<EnttObserver,
           boost::mp11::mp_size<InRangeUpdateComponentTypes>::value>
    inRangeUpdateObservers{};
std::array<EnttObserver, boost::mp11::mp_size<InRangeInitComponentTypes>::value>
    inRangeDestroyObservers{};

// Check that every Update component is also in the Init list.
static_assert(
    boost::mp11::mp_all_of_q<
        SelfUpdateComponentTypes,
        boost::mp11::mp_bind<boost::mp11::mp_contains, SelfInitComponentTypes,
                             boost::mp11::_1>>::value,
    "Every component in SelfUpdateComponentTypes must also be in "
    "SelfInitComponentTypes.");
static_assert(
    boost::mp11::mp_all_of_q<InRangeUpdateComponentTypes,
                             boost::mp11::mp_bind<boost::mp11::mp_contains,
                                                  InRangeInitComponentTypes,
                                                  boost::mp11::_1>>::value,
    "Every component in InRangeUpdateComponentTypes must also be in "
    "InRangeInitComponentTypes.");

ComponentSyncSystem::ComponentSyncSystem(const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, graphicData{inSimContext.graphicData}
{
    // Self-Init
    boost::mp11::mp_for_each<SelfInitComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<SelfInitComponentTypes, ComponentType>::value};

        selfConstructObservers[typeIndex].bind(world.registry);
        selfConstructObservers[typeIndex]
            .template on_construct<ComponentType>();

        selfDestroyObservers[typeIndex].bind(world.registry);
        selfDestroyObservers[typeIndex].on_destroy<ComponentType>();
    });

    // Self-Update
    boost::mp11::mp_for_each<SelfUpdateComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<SelfUpdateComponentTypes,
                                 ComponentType>::value};

        selfUpdateObservers[typeIndex].bind(world.registry);
        selfUpdateObservers[typeIndex].template on_update<ComponentType>();
    });

    // In-Range-Init
    boost::mp11::mp_for_each<InRangeInitComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<InRangeInitComponentTypes,
                                 ComponentType>::value};

        // TODO: If a client is near an entity when it's constructed, it'll
        //       receive both an EntityInit and a ComponentUpdate. It'd be
        //       nice if we could find a way to just send one, but until then
        //       it isn't a huge cost.
        inRangeConstructObservers[typeIndex].bind(world.registry);
        inRangeConstructObservers[typeIndex]
            .template on_construct<ComponentType>();

        inRangeDestroyObservers[typeIndex].bind(world.registry);
        inRangeDestroyObservers[typeIndex].on_destroy<ComponentType>();
    });

    // In-Range-Update
    boost::mp11::mp_for_each<InRangeUpdateComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<InRangeUpdateComponentTypes,
                                 ComponentType>::value};

        inRangeConstructObservers[typeIndex].bind(world.registry);
        inRangeConstructObservers[typeIndex]
            .template on_update<ComponentType>();
    });
}

void ComponentSyncSystem::sendUpdates()
{
    ZoneScoped;

    sendSelfUpdates();

    sendInRangeUpdates();
}

void ComponentSyncSystem::sendSelfUpdates()
{
    entt::registry& registry{world.registry};

    // Add components to the each entity's ComponentUpdate.
    addConstructDestroyComponents<SelfInitComponentTypes>(
        selfConstructObservers, selfDestroyObservers);
    addUpdateComponents<SelfUpdateComponentTypes>(selfUpdateObservers);

    // Send the update to the each entity's client.
    for (auto& [updatedEntity, componentUpdate] : componentUpdateMap) {
        // If the entity doesn't exist anymore, skip it.
        if (!(registry.valid(updatedEntity))) {
            continue;
        }

        // Note: Every message in the map should be for a client entity, since 
        //       we check for it above.
        AM_ASSERT(registry.all_of<ClientSimData>(updatedEntity),
                  "Somehow added a message for a non-client entity.");
        const ClientSimData& client{registry.get<ClientSimData>(updatedEntity)};

        // Serialize the message.
        componentUpdate.entity = updatedEntity;
        componentUpdate.tickNum = simulation.getCurrentTick();
        BinaryBufferSharedPtr message{network.serialize(componentUpdate)};

        // Send the message.
        network.send(client.netID, message, componentUpdate.tickNum);
    }

    componentUpdateMap.clear();
}

void ComponentSyncSystem::sendInRangeUpdates()
{
    entt::registry& registry{world.registry};

    // TODO: We build a message for each updated entity, even if it doesn't
    //       exist anymore or there aren't any clients nearby to send it to.
    //       There may be ways to optimize by making it client-by-client like
    //       MovementSyncSystem.
    // Add components to the each entity's ComponentUpdate.
    addConstructDestroyComponents<InRangeInitComponentTypes>(
        inRangeConstructObservers, inRangeDestroyObservers);
    addUpdateComponents<InRangeUpdateComponentTypes>(inRangeUpdateObservers);

    // Send each update to all nearby clients.
    auto view{registry.view<Position, ClientSimData>()};
    for (auto& [updatedEntity, componentUpdate] : componentUpdateMap) {
        // If the entity doesn't exist anymore, skip it.
        if (!(world.registry.valid(updatedEntity))) {
            continue;
        }

        // Serialize the message.
        componentUpdate.entity = updatedEntity;
        componentUpdate.tickNum = simulation.getCurrentTick();
        BinaryBufferSharedPtr message{network.serialize(componentUpdate)};

        // Get the list of entities that are in range of the updated entity.
        const std::vector<entt::entity>* entitiesInRange{nullptr};
        if (const auto* client
            = registry.try_get<ClientSimData>(updatedEntity)) {
            // Clients already have their AOI list built.
            entitiesInRange = &(client->entitiesInAOI);
        }
        else {
            const auto& updatedEntityPosition{
                world.registry.get<Position>(updatedEntity)};
            entitiesInRange = &(world.entityLocator.getEntities(
                Cylinder{updatedEntityPosition, SharedConfig::AOI_RADIUS,
                         SharedConfig::AOI_HALF_HEIGHT}));
        }

        // Send the update to all nearby clients.
        for (entt::entity entity : *entitiesInRange) {
            if (view.contains(entity)) {
                const auto& client{view.get<ClientSimData>(entity)};
                network.send(client.netID, message, componentUpdate.tickNum);
            }
        }
    }

    componentUpdateMap.clear();
}

template<typename ComponentTypeList>
void ComponentSyncSystem::addConstructDestroyComponents(
    auto& constructObservers, auto& destroyObservers)
{
    entt::registry& registry{world.registry};

    boost::mp11::mp_for_each<ComponentTypeList>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t observedTypeIndex{
            boost::mp11::mp_find<ComponentTypeList, ComponentType>::value};
        constexpr std::size_t replicatedTypeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes,
                                 ComponentType>::value};

        // For each entity that has a constructed component of this type, push
        // the component into the entity's message.
        for (entt::entity entity : constructObservers[observedTypeIndex]) {
            // If the entity isn't a client entity, skip it.
            // If the entity no longer has this component (it was constructed/
            // destructed on the same tick), don't send it.
            if (!registry.all_of<IsClientEntity, ComponentType>(entity)) {
                continue;
            }

            if constexpr (std::is_empty_v<ComponentType>) {
                // Note: Can't registry.get() empty types.
                componentUpdateMap[entity].updatedComponents.push_back(
                    ComponentType{});
            }
            else {
                const auto& component{registry.get<ComponentType>(entity)};
                componentUpdateMap[entity].updatedComponents.emplace_back(
                    component);
            }
        }

        // For each entity that has a destroyed component of this type, push
        // the component into the entity's message.
        for (entt::entity entity : destroyObservers[observedTypeIndex]) {
            // If the entity isn't a client entity, skip it.
            if (!registry.all_of<IsClientEntity>(entity)) {
                continue;
            }

            // Note: The message uses the index from ReplicatedComponentTypes.
            componentUpdateMap[entity].destroyedComponents.emplace_back(
                static_cast<Uint8>(replicatedTypeIndex));
        }

        constructObservers[observedTypeIndex].clear();
        destroyObservers[observedTypeIndex].clear();
    });
}

template<typename ComponentTypeList>
void ComponentSyncSystem::addUpdateComponents(auto& updateObservers)
{
    entt::registry& registry{world.registry};

    boost::mp11::mp_for_each<ComponentTypeList>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t observedTypeIndex{
            boost::mp11::mp_find<ComponentTypeList, ComponentType>::value};
        constexpr std::size_t replicatedTypeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes,
                                 ComponentType>::value};

        // For each entity that has an updated component of this type, push
        // the component into the entity's message.
        for (entt::entity entity : updateObservers[observedTypeIndex]) {
            // If the entity isn't a client entity, skip it.
            // If the entity no longer has this component (it was constructed/
            // destructed on the same tick), don't send it.
            if (!registry.all_of<IsClientEntity, ComponentType>(entity)) {
                continue;
            }

            if constexpr (std::is_empty_v<ComponentType>) {
                // Note: Can't registry.get() empty types.
                componentUpdateMap[entity].updatedComponents.push_back(
                    ComponentType{});
            }
            else {
                const auto& component{registry.get<ComponentType>(entity)};
                componentUpdateMap[entity].updatedComponents.emplace_back(
                    component);
            }
        }

        updateObservers[observedTypeIndex].clear();
    });
}

} // namespace Server
} // namespace AM
