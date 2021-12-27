#pragma once

#include "QueuedEvents.h"
#include "EntityInit.h"
#include "EntityDelete.h"
#include <queue>

namespace AM
{
namespace Client
{
class Simulation;
class World;
class SpriteData;

/**
 * Maintains the entity registry, constructing and deleting entities based
 * on messages from the server.
 */
class NpcLifetimeSystem
{
public:
    NpcLifetimeSystem(Simulation& inSim, World& inWorld, SpriteData& inSpriteData, EventDispatcher& inNetworkEventDispatcher);

    /**
     * Processes any waiting EntityInit or EntityDelete messages.
     */
    void processUpdates();

    /**
     * Applies the given tick adjustment (received from the server) to
     * tickReplicationOffset.
     */
    void applyTickAdjustment(int adjustment);

private:
    /**
     * Processes waiting EntityDelete messages, up to desiredTick.
     */
    void processEntityDeletes(Uint32 desiredTick);

    /**
     * Processes waiting EntityInit messages, up to desiredTick.
     */
    void processEntityInits(Uint32 desiredTick);

    /** Used to get the current tick number. */
    Simulation& sim;
    /** Used to access components. */
    World& world;
    /** Used to get sprite data when constructing entities. */
    SpriteData& spriteData;

    EventQueue<EntityInit> entityInitQueue;
    EventQueue<EntityDelete> entityDeleteQueue;

    /**
     * How far into the past to replicate NPCs at.
     * e.g. If tickReplicationOffset == -5, on tick 15 we'll replicate NPC data
     *      for tick 10.
     */
    int tickReplicationOffset;
};

} // End namespace Client
} // End namespace AM
