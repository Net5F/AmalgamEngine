#include "EntityLifetimeSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "Name.h"
#include "Input.h"
#include "InputHistory.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Collision.h"
#include "Rotation.h"
#include "EntityType.h"
#include "UserConfig.h"
#include "Camera.h"
#include "NeedsAdjacentChunks.h"
#include "SDLHelpers.h"
#include "Transforms.h"
#include "Config.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Client
{
EntityLifetimeSystem::EntityLifetimeSystem(Simulation& inSimulation, World& inWorld,
                                     SpriteData& inSpriteData,
                                     Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, spriteData{inSpriteData}
, clientEntityInitSecondaryQueue{}
, clientEntityInitQueue{inNetwork.getEventDispatcher()}
, dynamicObjectInitQueue{inNetwork.getEventDispatcher()}
, entityDeleteQueue{inNetwork.getEventDispatcher()}
{
}

void EntityLifetimeSystem::processUpdates()
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

void EntityLifetimeSystem::processEntityDeletes(Uint32 desiredTick)
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

void EntityLifetimeSystem::processClientEntityInits(Uint32 desiredTick)
{
    // Immediately process any player entity messages, push the rest into a 
    // secondary queue.
    {
        ClientEntityInit entityInit{};
        while (clientEntityInitQueue.pop(entityInit)) {
            if (entityInit.entity == world.playerEntity) {
                processClientEntityInit(entityInit);
            }
            else {
                clientEntityInitSecondaryQueue.push(entityInit);
            }
        }
    }

    // Process all messages in the secondary queue.
    while (!(clientEntityInitSecondaryQueue.empty())) {
        ClientEntityInit& entityInit{clientEntityInitSecondaryQueue.front()};

        // If we've reached the desired tick, save the rest of the messages for 
        // later.
        if (entityInit.tickNum > desiredTick) {
            break;
        }

        // Process the message.
        processClientEntityInit(entityInit);

        clientEntityInitSecondaryQueue.pop();
    }
}

void EntityLifetimeSystem::processClientEntityInit(const ClientEntityInit& entityInit)
{
    entt::registry& registry{world.registry};

    // Create the entity and construct its standard components.
    entt::entity newEntity{registry.create(entityInit.entity)};
    if (newEntity != entityInit.entity) {
        LOG_FATAL("Created entity doesn't match received entity. "
                  "Created: %u, received: %u",
                  newEntity, entityInit.entity);
    }

    // Note: Be careful with holding onto references here. If components 
    //       are added to the same group, the ref will be invalidated.
    registry.emplace<EntityType>(newEntity, EntityType::ClientEntity);
    registry.emplace<Name>(newEntity, entityInit.name);

    // Note: These will be set for real when we get their first movement
    //       update.
    registry.emplace<Input>(newEntity);
    registry.emplace<Position>(newEntity, entityInit.position);
    registry.emplace<PreviousPosition>(newEntity, entityInit.position);
    registry.emplace<Velocity>(newEntity);
    registry.emplace<Rotation>(newEntity, entityInit.rotation);

    // TODO: When we add character sprite sets, update this.
    Uint16 spriteSetID{spriteData
            .getObjectSpriteSet(SharedConfig::DEFAULT_CHARACTER_SPRITE_SET)
            .numericID};
    const auto& animationState{registry.emplace<AnimationState>(
        newEntity, SpriteSet::Type::Object, spriteSetID,
        SharedConfig::DEFAULT_CHARACTER_SPRITE_INDEX)};
    const Sprite* sprite{
        spriteData.getObjectSpriteSet(animationState.spriteSetID)
            .sprites[animationState.spriteIndex]};
    registry.emplace<Sprite>(newEntity, *sprite);

    // Note: Every entity needs a Collision because there may be cases where 
    //       they collide with something, but the collision logic doesn't 
    //       let e.g. players collide with other players.
    registry.emplace<Collision>(
        newEntity, sprite->modelBounds,
        Transforms::modelToWorldCentered(
            sprite->modelBounds, registry.get<Position>(newEntity)));

    // If we just added the player entity, we need to do some extra steps.
    if (newEntity == world.playerEntity) {
        finishPlayerEntity();
        LOG_INFO("Player entity added: %u. Message tick: %u", newEntity,
                 entityInit.tickNum);
    }
    else {
        LOG_INFO("Peer entity added: %u. Message tick: %u", newEntity,
                 entityInit.tickNum);
    }
}

void EntityLifetimeSystem::finishPlayerEntity()
{
    entt::registry& registry{world.registry};
    entt::entity playerEntity{world.playerEntity};

    // TODO: Switch to logical screen size and do scaling in Renderer.
    UserConfig& userConfig{UserConfig::get()};
    registry.emplace<Camera>(
        playerEntity, Camera::CenterOnEntity, Position{}, PreviousPosition{},
        SDLHelpers::rectToFRect(userConfig.getWindowSize()));

    registry.emplace<InputHistory>(playerEntity);

    // Flag that we need to request all map data.
    registry.emplace<NeedsAdjacentChunks>(playerEntity);
}

void EntityLifetimeSystem::processDynamicObjectInits(Uint32 desiredTick)
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

        const AnimationState& animationState{objectInit->animationState};
        registry.emplace<AnimationState>(newEntity, animationState);

        // Note: Unlike the server, we add a Sprite component to dynamic 
        //       entities. This lets the rendering system pick them up.
        const Sprite* sprite{
            spriteData.getObjectSpriteSet(animationState.spriteSetID)
                .sprites[animationState.spriteIndex]};
        registry.emplace<Sprite>(newEntity, *sprite);

        registry.emplace<Collision>(
            newEntity, sprite->modelBounds,
            Transforms::modelToWorldCentered(
                sprite->modelBounds, registry.get<Position>(newEntity)));

        registry.emplace<Interaction>(newEntity, objectInit->interaction);

        LOG_INFO("Dynamic object entity added: %u. Desired tick: %u, Message "
                 "tick: %u",
                 newEntity, desiredTick, objectInit->tickNum);

        dynamicObjectInitQueue.pop();
        objectInit = dynamicObjectInitQueue.peek();
    }
}

} // End namespace Client
} // End namespace AM
