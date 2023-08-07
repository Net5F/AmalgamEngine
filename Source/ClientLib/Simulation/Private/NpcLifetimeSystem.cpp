#include "NpcLifetimeSystem.h"
#include "Simulation.h"
#include "World.h"
#include "SpriteData.h"
#include "Name.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Collision.h"
#include "Rotation.h"
#include "IsClientEntity.h"
#include "Sprite.h"
#include "Config.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Client
{
NpcLifetimeSystem::NpcLifetimeSystem(Simulation& inSimulation, World& inWorld,
                                     SpriteData& inSpriteData,
                                     EventDispatcher& inNetworkEventDispatcher)
: simulation{inSimulation}
, world{inWorld}
, spriteData{inSpriteData}
, clientEntityInitQueue{inNetworkEventDispatcher}
, nonClientEntityInitQueue{inNetworkEventDispatcher}
, entityDeleteQueue{inNetworkEventDispatcher}
{
}

void NpcLifetimeSystem::processUpdates()
{
    // We want to process updates until we've either processed the desired
    // tick, or have run out of data.
    Uint32 desiredTick{simulation.getReplicationTick()};

    // Process any waiting EntityDelete messages, up to desiredTick.
    processEntityDeletes(desiredTick);

    // Process any waiting ClientEntityInit messages, up to desiredTick.
    processClientEntityInits(desiredTick);

    // Process any waiting NonClientEntityInit messages, up to desiredTick.
    processNonClientEntityInits(desiredTick);
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
            LOG_FATAL("Asked to delete invalid entity: %u",
                      entityDelete->entity);
        }

        entityDeleteQueue.pop();
        entityDelete = entityDeleteQueue.peek();
    }
}

void NpcLifetimeSystem::processClientEntityInits(Uint32 desiredTick)
{
    entt::registry& registry{world.registry};

    // Construct the client entities that entered our AOI on this tick (or 
    // previous ticks).
    ClientEntityInit* entityInit{clientEntityInitQueue.peek()};
    while ((entityInit != nullptr) && (entityInit->tickNum <= desiredTick)) {
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
            registry.emplace<Position>(entityInit->entity,
                                       entityInit->position);
            registry.emplace<PreviousPosition>(entityInit->entity,
                                               entityInit->position);
            registry.emplace<Velocity>(entityInit->entity);
            registry.emplace<Rotation>(entityInit->entity);
            registry.emplace<IsClientEntity>(newEntity);

            // Construct their name using the received name.
            registry.emplace<Name>(entityInit->entity, entityInit->name);

            // Construct their sprite using the received numericID.
            const Sprite& sprite{spriteData.getSprite(entityInit->numericID)};
            registry.emplace<Sprite>(entityInit->entity, sprite);

            // Construct their collision (we don't need to position it until 
            // they move, since the player entity doesn't collide with them).
            registry.emplace<Collision>(entityInit->entity, sprite.modelBounds,
                                        BoundingBox{});

            LOG_INFO("Entity added: %u. Desired tick: %u, Message tick: %u",
                     entityInit->entity, desiredTick, entityInit->tickNum);
        }
        else {
            LOG_FATAL("Asked to construct entity that already exists: %u",
                      entityInit->entity);
        }

        clientEntityInitQueue.pop();
        entityInit = clientEntityInitQueue.peek();
    }
}

void NpcLifetimeSystem::processNonClientEntityInits(Uint32 desiredTick)
{
    entt::registry& registry{world.registry};

    // Construct the non-client entities that entered our AOI on this tick (or 
    // previous ticks).
    NonClientEntityInit* entityInit{nonClientEntityInitQueue.peek()};
    while ((entityInit != nullptr) && (entityInit->tickNum <= desiredTick)) {
        if (!(registry.valid(entityInit->entity))) {
            // Create the entity.
            entt::entity newEntity{registry.create(entityInit->entity)};
            if (entityInit->entity != newEntity) {
                LOG_FATAL("Created entity doesn't match received entity. "
                          "Created: %u, received: %u",
                          newEntity, entityInit->entity);
            }

            // Construct their Position.
            registry.emplace<Position>(entityInit->entity,
                                       entityInit->position);

            // Construct their sprite using the received numericID.
            const Sprite& sprite{spriteData.getSprite(entityInit->numericID)};
            registry.emplace<Sprite>(entityInit->entity, sprite);

            // TODO: Construct components from vector.

            LOG_INFO("Entity added: %u. Desired tick: %u, Message tick: %u",
                     entityInit->entity, desiredTick, entityInit->tickNum);
        }
        else {
            LOG_FATAL("Asked to construct entity that already exists: %u",
                      entityInit->entity);
        }

        nonClientEntityInitQueue.pop();
        entityInit = nonClientEntityInitQueue.peek();
    }
}

} // End namespace Client
} // End namespace AM
