#pragma once

#include "Rotation.h"
#include "EntityInit.h"
#include "EntityDelete.h"
#include "QueuedEvents.h"
#include <queue>

namespace AM
{
struct EntityGraphicSet;

namespace Client
{
struct SimulationContext;
class Simulation;
class World;
class Network;
class GraphicData;

/**
 * Maintains the entity registry, constructing and deleting entities based
 * on messages from the server.
 */
class EntityLifetimeSystem
{
public:
    EntityLifetimeSystem(const SimulationContext& inSimContext);

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
    void processEntityData(Uint32 tickNum,
                           const EntityInit::EntityData& entityData);

    /**
     * Handles any processing that's specific to the player entity.
     */
    void finishPlayerEntity();

    /** Used to get the current tick number. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to get graphics data when constructing entities. */
    GraphicData& graphicData;

    /** We pop messages off entityInitQueue and push them into here, so
        we can find and immediately process any messages for the player
        entity. */
    std::queue<EntityInit> entityInitSecondaryQueue;

    EventQueue<EntityInit> entityInitQueue;
    EventQueue<EntityDelete> entityDeleteQueue;
};

} // End namespace Client
} // End namespace AM
