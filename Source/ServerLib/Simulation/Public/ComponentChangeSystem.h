#pragma once

#include "NameChangeRequest.h"
#include "AnimationStateChangeRequest.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Server
{
class World;
class Network;
class SpriteData;
class ISimulationExtension;

/**
 * Handles change requests for any random engine components that aren't already 
 * handled by another system.
 *
 * We do this so that the project can just send a change message, instead of 
 * needing to roll its own system.
 */
class ComponentChangeSystem
{
public:
    ComponentChangeSystem(World& inWorld, Network& inNetwork,
                          SpriteData& inSpriteData);

    /**
     * Processes any waiting component change request messages.
     */
    void processChangeRequests();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * Updates Sprite and Collision components when an AnimationState is
     * updated. Neither component is replicated, so we need to maintain them
     * ourselves.
     */
    void onAnimationStateUpdated(entt::registry& registry, entt::entity entity);

    /** Used for fetching component data. */
    World& world;
    /** Used for receiving updates from clients. */
    Network& network;
    /** Used to update Collision components when AnimationState is updated. */
    SpriteData& spriteData;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if component update requests are valid. */
    ISimulationExtension* extension;

    EventQueue<NameChangeRequest> nameChangeRequestQueue;
    EventQueue<AnimationStateChangeRequest> animationStateChangeRequestQueue;
};

} // namespace Server
} // namespace AM
