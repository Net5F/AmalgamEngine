#include "ComponentSyncSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "EngineComponentLists.h"
#include "ProjectComponentLists.h"
#include "ReplicatedComponent.h"
#include "ClientSimData.h"
#include "Cylinder.h"
#include "SharedConfig.h"
#include "Log.h"
#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/map.hpp"
#include "boost/mp11/bind.hpp"

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
using ObservedComponentMap
    = boost::mp11::mp_append<EngineComponentLists::ObservedComponentMap,
                             ProjectComponentLists::ObservedComponentMap>;
using ObservedComponents = boost::mp11::mp_map_keys<ObservedComponentMap>;
static_assert(boost::mp11::mp_is_map<ObservedComponentMap>::value,
              "One of the ObservedComponentMaps has invalid syntax.");
static_assert(
    boost::mp11::mp_all_of_q<
        boost::mp11::mp_second<ObservedComponentMap>,
        boost::mp11::mp_bind_front<boost::mp11::mp_contains,
                                   ReplicatedComponentTypes>>::value,
    "One of the SendComponents was not found in ReplicatedComponentTypes.");

/** A group and update observer for each observed component type. */
std::array<entt::observer, boost::mp11::mp_size<ObservedComponents>::value>
    observers;

//-----------------------------------------------------------------------------
// ComponentSyncSystem members
//-----------------------------------------------------------------------------
ComponentSyncSystem::ComponentSyncSystem(Simulation& inSimulation, World& inWorld,
                                       Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
{
    boost::mp11::mp_for_each<ObservedComponents>([&](auto I) {
        using ObservedComponent = decltype(I);
        constexpr std::size_t index{
            boost::mp11::mp_find<ObservedComponents, ObservedComponent>::value};

        observers[index].connect(world.registry,
                                 entt::collector.group<ObservedComponent>()
                                     .update<ObservedComponent>());
    });
}

// TODO: Make sure to emplace_or_replace on the client side, since we might be 
//       sending a component that wasn't in the init
void ComponentSyncSystem::sendUpdates()
{
    // Build an EntityUpdate for each entity that has an updated component.
    boost::mp11::mp_for_each<ObservedComponents>([&](auto I) {
        using ObservedComponent = decltype(I);
        using SendComponents
            = boost::mp11::mp_map_find<ObservedComponentMap, ObservedComponent>;
        constexpr std::size_t index{
            boost::mp11::mp_find<ObservedComponents, ObservedComponent>::value};

        // For each entity that was updated, push its components into its 
        // message.
        for (entt::entity entity : observers[index]) {
            boost::mp11::mp_for_each<SendComponents>([&](auto I) {
                const auto& component{world.registry.get<decltype(I)>(entity)};
                componentUpdateMap[entity].components.push_back(component);
            });
        }

        observers[index].clear();
    });

    // Send each update to all nearby entities.
    auto view{world.registry.view<Position, ClientSimData>()};
    for (auto& [entity, componentUpdate] : componentUpdateMap) {
        // Serialize the message.
        BinaryBufferSharedPtr message{network.serialize(componentUpdate)};

        // Send the update to all nearby clients.
        auto [position, client] = view.get<Position, ClientSimData>(entity);
        const auto& entitiesInRange{world.entityLocator.getEntities(
            {position, SharedConfig::AOI_RADIUS})};
        for (entt::entity entity : entitiesInRange) {
            network.send(client.netID, message);
        }
    }

    componentUpdateMap.clear();
}

} // namespace Server
} // namespace AM
