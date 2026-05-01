#include "ComponentSyncSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "Network.h"
#include "GraphicData.h"
#include "ReplicatedComponent.h"
#include "EnttObserver.h"
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

// TODO: Add a check that all components in ObserveBroadcast are also in 
//       InRangeInit

// TODO: If we're going to do this, we'll need to split construct into a third 
//       set of observers. Is that fine?
//       Destroy uses init list, right?

/** Init-Self, Update-Self Observers. */
using InitSelfComponentTypes
    = boost::mp11::mp_append<EngineInitSelfComponentTypes,
                             ProjectInitSelfComponentTypes>;
using UpdateSelfComponentTypes
    = boost::mp11::mp_append<EngineUpdateSelfComponentTypes,
                             ProjectUpdateSelfComponentTypes>;
std::array<EnttObserver, boost::mp11::mp_size<InitSelfComponentTypes>::value>
    constructSelfObservers{};
std::array<EnttObserver, boost::mp11::mp_size<UpdateSelfComponentTypes>::value>
    updateSelfObservers{};
std::array<EnttObserver, boost::mp11::mp_size<InitSelfComponentTypes>::value>
    destroySelfObservers{};

using ObserveSelfComponentTypes
    = boost::mp11::mp_append<EngineObserveSelfComponentTypes,
                             ProjectObserveSelfComponentTypes>;
/** A construct and update observer for each self component type. */
std::array<EnttObserver, boost::mp11::mp_size<ObserveSelfComponentTypes>::value>
    updateSelfObservers{};
/** A destruct observer for each self component type. */
std::array<EnttObserver, boost::mp11::mp_size<ObserveSelfComponentTypes>::value>
    destroySelfObservers{};



using ObserveBroadcastComponentTypes
    = boost::mp11::mp_append<EngineObserveBroadcastComponentTypes,
                             ProjectObserveBroadcastComponentTypes>;
/** A construct and update observer for each broadcast component type. */
std::array<EnttObserver,
           boost::mp11::mp_size<ObserveBroadcastComponentTypes>::value>
    updateBroadcastObservers{};
/** A destruct observer for each broadcast component type. */
std::array<EnttObserver,
           boost::mp11::mp_size<ObserveBroadcastComponentTypes>::value>
    destroyBroadcastObservers{};

using ObserveSelfComponentTypes
    = boost::mp11::mp_append<EngineObserveSelfComponentTypes,
                             ProjectObserveSelfComponentTypes>;
/** A construct and update observer for each self component type. */
std::array<EnttObserver, boost::mp11::mp_size<ObserveSelfComponentTypes>::value>
    updateSelfObservers{};
/** A destruct observer for each self component type. */
std::array<EnttObserver, boost::mp11::mp_size<ObserveSelfComponentTypes>::value>
    destroySelfObservers{};

ComponentSyncSystem::ComponentSyncSystem(const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, graphicData{inSimContext.graphicData}
{
    boost::mp11::mp_for_each<ObserveBroadcastComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<ObserveBroadcastComponentTypes,
                                 ComponentType>::value};

        // TODO: If a client is near an entity when it's constructed, it'll
        //       receive both an EntityInit and a ComponentUpdate (from the
        //       construct observer). It'd be nice if we could find a way to
        //       just send one, but until then it isn't a huge cost.
        updateBroadcastObservers[typeIndex].bind(world.registry);
        updateBroadcastObservers[typeIndex]
            .template on_construct<ComponentType>()
            .template on_update<ComponentType>();

        destroyBroadcastObservers[typeIndex].bind(world.registry);
        destroyBroadcastObservers[typeIndex].on_destroy<ComponentType>();
    });

    boost::mp11::mp_for_each<ObserveSelfComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<ObserveSelfComponentTypes,
                                 ComponentType>::value};

        updateSelfObservers[typeIndex].bind(world.registry);
        updateSelfObservers[typeIndex]
            .template on_construct<ComponentType>()
            .template on_update<ComponentType>();

        destroySelfObservers[typeIndex].bind(world.registry);
        destroySelfObservers[typeIndex].on_destroy<ComponentType>();
    });
}

void ComponentSyncSystem::sendUpdates()
{
    ZoneScoped;

    sendBroadcastUpdates();

    sendSelfUpdates();
}

void ComponentSyncSystem::sendBroadcastUpdates()
{
    entt::registry& registry{world.registry};

    // TODO: We build a message for each updated entity, even if it doesn't
    //       exist anymore or there aren't any clients nearby to send it to.
    //       There may be ways to optimize by making it client-by-client like
    //       MovementSyncSystem.
    // Build a ComponentUpdate for each entity that has constructed, updated,
    // or destroyed components.
    boost::mp11::mp_for_each<ObserveBroadcastComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t observedTypeIndex{
            boost::mp11::mp_find<ObserveBroadcastComponentTypes,
                                 ComponentType>::value};
        constexpr std::size_t replicatedTypeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes,
                                 ComponentType>::value};

        // For each entity that has a constructed or updated component of this
        // type, push the component into the entity's message.
        for (entt::entity entity :
             updateBroadcastObservers[observedTypeIndex]) {
            // If the entity no longer has this component (it was constructed/
            // destructed on the same tick), don't send it.
            if (!registry.all_of<ComponentType>(entity)) {
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
        // Note: The message uses the index from ReplicatedComponentTypes.
        for (entt::entity entity : destroyBroadcastObservers[observedTypeIndex]) {
            componentUpdateMap[entity].destroyedComponents.emplace_back(
                static_cast<Uint8>(replicatedTypeIndex));
        }

        updateBroadcastObservers[observedTypeIndex].clear();
        destroyBroadcastObservers[observedTypeIndex].clear();
    });

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

void ComponentSyncSystem::sendSelfUpdates()
{
    entt::registry& registry{world.registry};

    // Build a ComponentUpdate for each entity that has constructed, updated,
    // or destroyed components.
    boost::mp11::mp_for_each<ObserveSelfComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t observedTypeIndex{
            boost::mp11::mp_find<ObserveSelfComponentTypes,
                                 ComponentType>::value};
        constexpr std::size_t replicatedTypeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes,
                                 ComponentType>::value};

        // For each entity that has a constructed or updated component of this
        // type, push the component into the entity's message.
        for (entt::entity entity : updateSelfObservers[observedTypeIndex]) {
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
        for (entt::entity entity : destroySelfObservers[observedTypeIndex]) {
            // If the entity isn't a client entity, skip it.
            if (!registry.all_of<IsClientEntity>(entity)) {
                continue;
            }

            // Note: The message uses the index from ReplicatedComponentTypes.
            componentUpdateMap[entity].destroyedComponents.emplace_back(
                static_cast<Uint8>(replicatedTypeIndex));
        }

        updateSelfObservers[observedTypeIndex].clear();
        destroySelfObservers[observedTypeIndex].clear();
    });

    // Send the update to the entity's client.
    for (auto& [updatedEntity, componentUpdate] : componentUpdateMap) {
        // If the entity doesn't exist anymore, skip it.
        if (!(world.registry.valid(updatedEntity))) {
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

} // namespace Server
} // namespace AM
