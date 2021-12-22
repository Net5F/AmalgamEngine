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
class EntityLifespanSystem
{
public:
    EntityLifespanSystem(Simulation& inSim, World& inWorld, SpriteData& inSpriteData, EventDispatcher& inNetworkEventDispatcher);

    /**
     * Processes any waiting EntityInit or EntityDelete messages.
     */
    void processUpdates();

private:
    /** Used to get the current tick number. */
    Simulation& sim;
    /** Used to access components. */
    World& world;
    /** Used to get sprite data when constructing entities. */
    SpriteData& spriteData;

    EventQueue<EntityInit> entityInitQueue;
    EventQueue<EntityDelete> entityDeleteQueue;
};

} // End namespace Client
} // End namespace AM
