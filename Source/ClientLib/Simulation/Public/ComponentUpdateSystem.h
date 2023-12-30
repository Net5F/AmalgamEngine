#pragma once

#include "ComponentUpdate.h"
#include "QueuedEvents.h"
#include <queue>

namespace AM
{
namespace Client
{
class Simulation;
class World;
class Network;
class SpriteData;

/**
 * Applies received entity component updates.
 */
class ComponentUpdateSystem
{
public:
    ComponentUpdateSystem(Simulation& inSimulation, World& inWorld,
                          Network& inNetwork, SpriteData& inSpriteData);

    ~ComponentUpdateSystem();

    /**
     * Processes any received component updates.
     */
    void processUpdates();

private:
    void processComponentUpdate(const ComponentUpdate& componentUpdate);

    /**
     * Updates Sprite and Collision components when an AnimationState is
     * updated. Neither component is replicated, so we need to maintain them
     * ourselves.
     */
    void onAnimationStateUpdated(entt::registry& registry, entt::entity entity);

    /** Used to get the current replication tick. */
    Simulation& simulation;
    /** Used to access the components we need to update. */
    World& world;
    /** Used to receive component update messages. */
    Network& network;
    /** Used to update Sprite components when AnimationState is updated. */
    SpriteData& spriteData;

    /** We pop messages off componentUpdateQueue and push them into here, so
        we can find and immediately process any messages for the player
        entity. */
    std::queue<ComponentUpdate> componentUpdateSecondaryQueue;

    EventQueue<ComponentUpdate> componentUpdateQueue;
};

} // namespace Client
} // namespace AM
