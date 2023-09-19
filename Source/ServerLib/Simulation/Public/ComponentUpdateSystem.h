#pragma once

#include "ReplicatedComponent.h"
#include "ComponentUpdate.h"
#include "ComponentUpdateRequest.h"
#include "QueuedEvents.h"
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
class SpriteData;
class ISimulationExtension;

/**
 * Observes component updates based on the ObservedComponentTypes lists in 
 * EngineComponentLists.h and ProjectComponentLists.h.
 * When a component is updated, an update message will be sent to all nearby 
 * clients.
 */
class ComponentUpdateSystem
{
public:
    ComponentUpdateSystem(Simulation& inSimulation, World& inWorld,
                          Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Processes any waiting ComponentUpdateRequest messages.
     */
    void processUpdateRequests();

    /**
     * Sends updates for any observed components that were modified.
     */
    void sendUpdates();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * Updates Sprite and Collision components when an AnimationState is updated.
     * Neither component is replicated, so we need to maintain them ourselves.
     */
    void onAnimationStateUpdated(entt::registry& registry, entt::entity entity);

    /** Used to get the current tick number. */
    Simulation& simulation;
    /** Used for fetching component data. */
    World& world;
    /** Used for sending updates to clients. */
    Network& network;
    /** Used to update Collision components when AnimationState is updated. */
    SpriteData& spriteData;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if component update requests are valid. */
    ISimulationExtension* extension;

    // Note: Check the top of the cpp file for file-local types and variables.
    //       We keep some templated code there to reduce compile times.

    // Note: To optimize, we could store indices into a vector<ComponentUpdate>.
    //       Then we could re-use the vectors instead of re-allocating.
    /** Maps entityID -> a ComponentUpdate message containing that entity's data.
        We iterate the observers to detect changes, so this map lets us 
        iteratively build the update messages component-by-component. */
    std::unordered_map<entt::entity, ComponentUpdate> componentUpdateMap;

    EventQueue<ComponentUpdateRequest> componentUpdateRequestQueue;
};

} // namespace Server
} // namespace AM
