#pragma once

#include "ReplicatedComponent.h"
#include "ComponentUpdate.h"
#include "entt/fwd.hpp"
#include "entt/entity/registry.hpp"
#include <unordered_map>

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;
class SpriteData;

/**
 * Observes component updates based on the ObservedComponentTypes lists in
 * EngineComponentLists.h and ProjectComponentLists.h.
 *
 * When an observed component is updated, sends an update message to all nearby
 * clients.
 */
class ComponentSyncSystem
{
public:
    ComponentSyncSystem(Simulation& inSimulation, World& inWorld,
                        Network& inNetwork, SpriteData& inSpriteData);

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
    /** Used to update Collision components when AnimationState is updated. */
    SpriteData& spriteData;

    // Note: Check the top of the cpp file for file-local types and variables.
    //       We keep some templated code there to reduce compile times.

    // Note: To optimize, we could store indices into a vector<ComponentUpdate>.
    //       Then we could re-use the vectors instead of re-allocating.
    /** Maps entityID -> a ComponentUpdate message containing that entity's
       data. We iterate the observers to detect changes, so this map lets us
        iteratively build the update messages component-by-component. */
    std::unordered_map<entt::entity, ComponentUpdate> componentUpdateMap;
};

} // namespace Server
} // namespace AM
