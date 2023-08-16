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
#include "EntityType.h"
#include "Transforms.h"
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
, dynamicObjectInitQueue{inNetworkEventDispatcher}
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

    // Process any waiting init messages, up to desiredTick.
    processClientEntityInits(desiredTick);
    processDynamicObjectInits(desiredTick);
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
        // Create the entity and construct its standard components.
        entt::entity newEntity{registry.create(entityInit->entity)};
        if (newEntity != entityInit->entity) {
            LOG_FATAL("Created entity doesn't match received entity. "
                      "Created: %u, received: %u",
                      newEntity, entityInit->entity);
        }

        // Note: Be careful with holding onto references here. If components 
        //       are added to the same group, the ref will be invalidated.
        registry.emplace<EntityType>(newEntity, EntityType::ClientEntity);
        registry.emplace<Name>(newEntity, entityInit->name);

        // Note: These will be set for real when we get their first movement
        //       update.
        registry.emplace<Input>(newEntity);
        registry.emplace<Position>(newEntity, entityInit->position);
        registry.emplace<PreviousPosition>(newEntity, entityInit->position);
        registry.emplace<Velocity>(newEntity);
        registry.emplace<Rotation>(newEntity, entityInit->rotation);

        const Sprite& sprite{spriteData.getSprite(entityInit->numericID)};
        registry.emplace<Sprite>(newEntity, sprite);

        // Note: We don't need to position their collision until they move,
        //       since the player entity doesn't collide with them.
        registry.emplace<Collision>(newEntity, sprite.modelBounds,
                                    BoundingBox{});

        LOG_INFO("Peer entity added: %u. Desired tick: %u, Message tick: %u",
                 newEntity, desiredTick, entityInit->tickNum);

        clientEntityInitQueue.pop();
        entityInit = clientEntityInitQueue.peek();
    }
}

void NpcLifetimeSystem::processDynamicObjectInits(Uint32 desiredTick)
{
    entt::registry& registry{world.registry};

    // Construct the dynamic objects that entered our AOI on this tick (or 
    // previous ticks).
    DynamicObjectInit* objectInit{dynamicObjectInitQueue.peek()};
    while ((objectInit != nullptr) && (objectInit->tickNum <= desiredTick)) {
        // Create the entity and construct its standard components.
        entt::entity newEntity{registry.create(objectInit->entity)};
        if (newEntity != objectInit->entity) {
            LOG_FATAL("Created entity doesn't match received entity. "
                      "Created: %u, received: %u",
                      newEntity, objectInit->entity);
        }

        // Note: Be careful with holding onto references here. If components 
        //       are added to the same group, the ref will be invalidated.
        registry.emplace<EntityType>(newEntity, EntityType::DynamicObject);
        registry.emplace<Name>(newEntity, objectInit->name);

        registry.emplace<Position>(newEntity, objectInit->position);
        registry.emplace<Rotation>(newEntity, objectInit->rotation);

        const ObjectSpriteSet& spriteSet{
            spriteData.getObjectSpriteSet(objectInit->spriteSetID)};
        registry.emplace<ObjectSpriteSet>(newEntity, spriteSet);
        const Sprite& sprite{
            *(spriteSet.sprites[objectInit->rotation.direction])};
        registry.emplace<Sprite>(newEntity, sprite);

        registry.emplace<Collision>(
            newEntity, sprite.modelBounds,
            Transforms::modelToWorldCentered(
                sprite.modelBounds, registry.get<Position>(newEntity)));

        LOG_INFO("Dynamic object entity added: %u. Desired tick: %u, Message "
                 "tick: %u",
                 newEntity, desiredTick, objectInit->tickNum);

        dynamicObjectInitQueue.pop();
        objectInit = dynamicObjectInitQueue.peek();
    }
}

} // End namespace Client
} // End namespace AM
