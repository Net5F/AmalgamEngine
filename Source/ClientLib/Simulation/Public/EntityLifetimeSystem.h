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
class Network;
class SpriteData;

/**
 * Maintains the entity registry, constructing and deleting entities based
 * on messages from the server.
 */
class EntityLifetimeSystem
{
public:
    EntityLifetimeSystem(Simulation& inSimulation, World& inWorld,
                      SpriteData& inSpriteData, Network& inNetwork);

    /**
     * Processes any waiting EntityInit or EntityDelete messages.
     */
    void processUpdates();

private:
    /**
     * Processes waiting EntityDelete messages, up to desiredTick.
     */
    void processEntityDeletes(Uint32 desiredTick);

    /**
     * Processes waiting EntityInit messages, up to desiredTick.
     */
    void processEntityInits(Uint32 desiredTick);
    void processEntityInit(const EntityInit& entityInit);

    /**
     * Handles any processing that's specific to the player entity.
     */
    void finishPlayerEntity();

    /** Used to get the current tick number. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to get sprite data when constructing entities. */
    SpriteData& spriteData;

    /** We pop messages off entityInitQueue and push them into here, so 
        we can find and immediately process any messages for the player 
        entity. */
    std::queue<EntityInit> entityInitSecondaryQueue;

    EventQueue<EntityInit> entityInitQueue;
    EventQueue<EntityDelete> entityDeleteQueue;
};

} // End namespace Client
} // End namespace AM