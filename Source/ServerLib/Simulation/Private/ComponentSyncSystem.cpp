#include "ComponentSyncSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "EngineComponentLists.h"
#include "ProjectComponentLists.h"
#include "ReplicatedComponent.h"
#include "ClientSimData.h"
#include "Cylinder.h"
#include "Collision.h"
#include "SharedConfig.h"
#include "Log.h"
#include "entt/entity/observer.hpp"
#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/map.hpp"
#include "boost/mp11/bind.hpp"
#include "tracy/Tracy.hpp"

namespace AM
{
namespace Server
{

//-----------------------------------------------------------------------------
// Templated type setup
//-----------------------------------------------------------------------------
/**
 * See comment in EngineComponentLists.h
 */
using ObservedComponentTypes
    = boost::mp11::mp_append<EngineComponentLists::ObservedComponentTypes,
                             ProjectComponentLists::ObservedComponentTypes>;

/** A group and update observer for each observed component type. */
std::array<entt::observer, boost::mp11::mp_size<ObservedComponentTypes>::value>
    observers{};

//-----------------------------------------------------------------------------
// ComponentSyncSystem members
//-----------------------------------------------------------------------------
ComponentSyncSystem::ComponentSyncSystem(Simulation& inSimulation,
                                         World& inWorld, Network& inNetwork,
                                         SpriteData& inSpriteData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
{
    boost::mp11::mp_for_each<ObservedComponentTypes>([&](auto I) {
        using ObservedComponent = decltype(I);
        constexpr std::size_t index{
            boost::mp11::mp_find<ObservedComponentTypes,
                                 ObservedComponent>::value};

        // TODO: If a client is near an entity when it's constructed, it'll 
        //       receive both an EntityInit and an EntityUpdate (from the group
        //       observer). It'd be nice if we could find a way to just send one,
        //       but until then it isn't a huge cost.
        observers[index].connect(world.registry,
                                 entt::collector.group<ObservedComponent>()
                                     .template update<ObservedComponent>());
    });
}

void ComponentSyncSystem::sendUpdates()
{
    ZoneScoped;

    entt::registry& registry{world.registry};

    // TODO: We build a message for each updated entity, even if there aren't 
    //       any clients nearby to send it to. There may be ways to optimize by
    //       making it client-by-client like MovementSyncSystem.
    // Build an EntityUpdate for each entity that has an updated component.
    boost::mp11::mp_for_each<ObservedComponentTypes>([&](auto I) {
        using ObservedComponent = decltype(I);
        constexpr std::size_t index{
            boost::mp11::mp_find<ObservedComponentTypes,
                                 ObservedComponent>::value};

        // For each entity that was updated, push its components into its 
        // message.
        for (entt::entity entity : observers[index]) {
            if constexpr (std::is_empty_v<ObservedComponent>) {
                // Note: Can't registry.get() empty types.
                componentUpdateMap[entity].components.push_back(
                    ObservedComponent{});
            }
            else {
                const auto& component{registry.get<ObservedComponent>(entity)};
                componentUpdateMap[entity].components.push_back(component);
            }
        }

        observers[index].clear();
    });

    // Send each update to all nearby clients.
    auto view{registry.view<Position, ClientSimData>()};
    for (auto& [updatedEntity, componentUpdate] : componentUpdateMap) {
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
            const auto& updatedEntityPosition{view.get<Position>(updatedEntity)};
            entitiesInRange = &(world.entityLocator.getEntities(
                {updatedEntityPosition, SharedConfig::AOI_RADIUS}));
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
