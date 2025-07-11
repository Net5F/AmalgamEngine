#include "ComponentSyncSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "GraphicData.h"
#include "EngineObservedComponentTypes.h"
#include "ProjectObservedComponentTypes.h"
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

/**
 * See comment in EngineObservedComponents.h
 */
using ObservedComponentTypes
    = boost::mp11::mp_append<EngineObservedComponentTypes,
                             ProjectObservedComponentTypes>;

/** A construct and update observer for each observed component type. */
std::array<EnttObserver, boost::mp11::mp_size<ObservedComponentTypes>::value>
    updateObservers{};

/** A destruct observer for each observed component type. */
std::array<EnttObserver, boost::mp11::mp_size<ObservedComponentTypes>::value>
    destroyObservers{};

ComponentSyncSystem::ComponentSyncSystem(Simulation& inSimulation,
                                         World& inWorld, Network& inNetwork,
                                         GraphicData& inGraphicData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, graphicData{inGraphicData}
{
    boost::mp11::mp_for_each<ObservedComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<ObservedComponentTypes,
                                 ComponentType>::value};

        // TODO: If a client is near an entity when it's constructed, it'll
        //       receive both an EntityInit and a ComponentUpdate (from the
        //       construct observer). It'd be nice if we could find a way to just
        //       send one, but until then it isn't a huge cost.
        updateObservers[typeIndex].bind(world.registry);
        updateObservers[typeIndex]
            .template on_construct<ComponentType>()
            .template on_update<ComponentType>();

        destroyObservers[typeIndex].bind(world.registry);
        destroyObservers[typeIndex].on_destroy<ComponentType>();
    });
}

void ComponentSyncSystem::sendUpdates()
{
    ZoneScoped;

    entt::registry& registry{world.registry};

    // TODO: We build a message for each updated entity, even if it doesn't 
    //       exist anymore or there aren't any clients nearby to send it to.
    //       There may be ways to optimize by making it client-by-client like 
    //       MovementSyncSystem.
    // Build an EntityUpdate for each entity that has constructed, updated, 
    // or destroyed components.
    boost::mp11::mp_for_each<ObservedComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        constexpr std::size_t observedTypeIndex{
            boost::mp11::mp_find<ObservedComponentTypes,
                                 ComponentType>::value};
        constexpr std::size_t replicatedTypeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes,
                                 ComponentType>::value};

        // For each entity that has a constructed or updated component of this 
        // type, push the component into the entity's message.
        for (entt::entity entity : updateObservers[observedTypeIndex]) {
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
        for (entt::entity entity : destroyObservers[observedTypeIndex]) {
            componentUpdateMap[entity].destroyedComponents.emplace_back(
                static_cast<Uint8>(replicatedTypeIndex));
        }

        updateObservers[observedTypeIndex].clear();
        destroyObservers[observedTypeIndex].clear();
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

} // namespace Server
} // namespace AM
