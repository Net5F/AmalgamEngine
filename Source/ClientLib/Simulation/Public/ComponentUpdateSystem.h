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
class GraphicData;

/**
 * Applies received entity component updates.
 */
class ComponentUpdateSystem
{
public:
    ComponentUpdateSystem(Simulation& inSimulation, World& inWorld,
                          Network& inNetwork, GraphicData& inGraphicData);

    ~ComponentUpdateSystem();

    /**
     * Processes any received component updates.
     */
    void processUpdates();

private:
    void processComponentUpdate(const ComponentUpdate& componentUpdate);

    /**
     * Updates Collision state when a GraphicState is updated.
     * Neither component is replicated, so we need to maintain them ourselves.
     */
    void onGraphicStateUpdated(entt::registry& registry, entt::entity entity);

    /**
     * Updates the entity's collision in CollisionLocator.
     */
    void onCollisionBitSetsUpdated(entt::registry& registry,
                                   entt::entity entity);

    /** Used to get the current replication tick. */
    Simulation& simulation;
    /** Used to access the components we need to update. */
    World& world;
    /** Used to receive component update messages. */
    Network& network;
    /** Used to update components when GraphicState is updated. */
    GraphicData& graphicData;

    /** We pop messages off componentUpdateQueue and push them into here, so
        we can find and immediately process any messages for the player
        entity. */
    std::queue<ComponentUpdate> componentUpdateSecondaryQueue;

    EventQueue<ComponentUpdate> componentUpdateQueue;
};

} // namespace Client
} // namespace AM
