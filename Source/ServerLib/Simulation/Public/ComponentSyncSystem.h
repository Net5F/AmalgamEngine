#pragma once

#include "ReplicatedComponent.h"
#include "ComponentUpdate.h"
#include "entt/fwd.hpp"
#include "entt/entity/registry.hpp"
#include "entt/entity/observer.hpp"
#include <array>
#include <unordered_map>

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;

/**
 * Observes component updates based on the ObservedComponentTypes lists in 
 * EngineComponentLists.h and ProjectComponentLists.h.
 * When a component is updated, an update message will be sent to all nearby 
 * clients.
 *
 * Note: We'll send updates for arbitrary components, but we intentionally avoid 
 *       creating a generic "change component" message for clients to use.
 *       If a client wants to change a component, the server should have to be 
 *       very particular in validating that request before applying it.
 */
class ComponentSyncSystem
{
public:
    ComponentSyncSystem(Simulation& inSimulation, World& inWorld,
                        Network& inNetwork);

    /**
     * Sends updates for any observed components that were modified.
     */
    void sendUpdates();

private:
    /** Used to get the current tick number. */
    Simulation& simulation;
    /** Used for fetching component data. */
    World& world;
    /** Used for sending updates to clients. */
    Network& network;

    // Note: Check the top of the cpp file for file-local types and variables.
    //       We keep some templated code there to reduce compile times.

    /** Maps entityID -> a ComponentInit message containing that entity's data.
        We iterate the observers to detect changes, so this map lets us 
        iteratively build the update messages component-by-component. */
    std::unordered_map<entt::entity, ComponentUpdate> componentUpdateMap;
};

} // namespace Server
} // namespace AM
