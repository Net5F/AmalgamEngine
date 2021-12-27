#include "NpcLifetimeSystem.h"
#include "Simulation.h"
#include "World.h"
#include "SpriteData.h"
#include "Name.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "Config.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Client
{

NpcLifetimeSystem::NpcLifetimeSystem(Simulation& inSim,
    World& inWorld,
    SpriteData& inSpriteData,
    EventDispatcher& inNetworkEventDispatcher)
: sim{inSim}
, world{inWorld}
, spriteData{inSpriteData}
, entityInitQueue{inNetworkEventDispatcher}
, entityDeleteQueue{inNetworkEventDispatcher}
, tickReplicationOffset(Config::INITIAL_REPLICATION_OFFSET)
{
}

void NpcLifetimeSystem::processUpdates()
{
    // We want to process updates until we've either processed the desired
    // tick, or run out of data.
    Uint32 desiredTick{sim.getCurrentTick() + tickReplicationOffset};

    // Process any waiting EntityDelete messages, up to desiredTick.
    processEntityDeletes(desiredTick);

    // Process any waiting EntityInit messages, up to desiredTick.
    processEntityInits(desiredTick);
}

void NpcLifetimeSystem::applyTickAdjustment(int adjustment)
{
    // We set our client ahead of the server by an amount equal to our latency,
    // but this means that received messages will appear to be doubly far into
    // the past.
    // To account for this, we double the adjustment before applying.
    // We also negate it since we're reversing the direction.
    tickReplicationOffset += (-2 * adjustment);

    if (tickReplicationOffset >= 0) {
        LOG_FATAL("Adjusted tickReplicationOffset too far into the future. "
                  "offset: %u",
                  tickReplicationOffset);
    }
}

void NpcLifetimeSystem::processEntityDeletes(Uint32 desiredTick)
{
    entt::registry& registry{world.registry};

    // Delete the entities that left our AOI on this tick (or previous ticks).
    EntityDelete* entityDelete{entityDeleteQueue.peek()};
    while ((entityDelete != nullptr)
           && (entityDelete->tickNum <= desiredTick)) {
        if (registry.valid(entityDelete->entity)) {
            registry.destroy(entityDelete->entity);

            LOG_INFO("Entity removed: %u. Desired tick: %u, Message tick: %u",
                entityDelete->entity, desiredTick, entityDelete->tickNum);
        }
        else {
            LOG_FATAL("Asked to delete invalid entity: %u", entityDelete->entity);
        }

        entityDeleteQueue.pop();
        entityDelete = entityDeleteQueue.peek();
    }
}

void NpcLifetimeSystem::processEntityInits(Uint32 desiredTick)
{
    entt::registry& registry{world.registry};

    // Construct the entities that entered our AOI on this tick (or previous
    // ticks).
    EntityInit* entityInit{entityInitQueue.peek()};
    while ((entityInit != nullptr)
           && (entityInit->tickNum <= desiredTick)) {
        if (!(registry.valid(entityInit->entity))) {
            // Create the entity.
            entt::entity newEntity{registry.create(entityInit->entity)};
            if (entityInit->entity != newEntity) {
                LOG_FATAL("Created entity doesn't match received entity. "
                          "Created: %u, received: %u",
                          newEntity, entityInit->entity);
            }

            // Construct their movement-related components (to be set for real
            // when we get an EntityUpdate).
            registry.emplace<Input>(entityInit->entity);
            registry.emplace<Position>(entityInit->entity);
            registry.emplace<PreviousPosition>(entityInit->entity);
            registry.emplace<Movement>(entityInit->entity);

            // Construct their name using the received name.
            registry.emplace<Name>(entityInit->entity, entityInit->name);

            // Construct their sprite using the received numericID.
            registry.emplace<Sprite>(entityInit->entity, spriteData.get(entityInit->numericID));

            // Construct a placeholder bounding box. (they'll get a real
            // bounding box once they get a position update.)
            registry.emplace<BoundingBox>(entityInit->entity, BoundingBox{});

            LOG_INFO("Entity added: %u. Desired tick: %u, Message tick: %u",
                entityInit->entity, desiredTick, entityInit->tickNum);
        }
        else {
            LOG_FATAL("Asked to construct entity that already exists: %u", entityInit->entity);
        }

        entityInitQueue.pop();
        entityInit = entityInitQueue.peek();
    }
}

} // End namespace Client
} // End namespace AM
