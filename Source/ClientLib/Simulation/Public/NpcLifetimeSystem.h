#pragma once

#include "QueuedEvents.h"
#include "ClientEntityInit.h"
#include "NonClientEntityInit.h"
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
 *
 * From this Client's perspective, NPCs are any entity that isn't controlled by 
 * this client's player. See IsClientEntity.h for more info.
 */
class NpcLifetimeSystem
{
public:
    NpcLifetimeSystem(Simulation& inSimulation, World& inWorld,
                      SpriteData& inSpriteData,
                      EventDispatcher& inNetworkEventDispatcher);

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
     * Processes waiting ClientEntityInit messages, up to desiredTick.
     */
    void processClientEntityInits(Uint32 desiredTick);

    /**
     * Processes waiting NonClientEntityInit messages, up to desiredTick.
     */
    void processNonClientEntityInits(Uint32 desiredTick);

    /** Used to get the current tick number. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to get sprite data when constructing entities. */
    SpriteData& spriteData;

    EventQueue<ClientEntityInit> clientEntityInitQueue;
    EventQueue<NonClientEntityInit> nonClientEntityInitQueue;
    EventQueue<EntityDelete> entityDeleteQueue;
};

} // End namespace Client
} // End namespace AM
