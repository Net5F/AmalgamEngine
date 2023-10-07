#include "EntityLifetimeSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "Name.h"
#include "PreviousPosition.h"
#include "Collision.h"
#include "UserConfig.h"
#include "Camera.h"
#include "InputHistory.h"
#include "NeedsAdjacentChunks.h"
#include "SDLHelpers.h"
#include "Transforms.h"
#include "MovementHelpers.h"
#include "Config.h"
#include "entt/entity/registry.hpp"
#include <variant>
#include <string_view>

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
, entityInitSecondaryQueue{}
, entityInitQueue{inNetwork.getEventDispatcher()}
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
    processEntityInits(desiredTick);
}

void EntityLifetimeSystem::processEntityDeletes(Uint32 desiredTick)
{
    entt::registry& registry{world.registry};

    // Delete the entities that left our AOI on this tick (or previous ticks).
    EntityDelete* entityDelete{entityDeleteQueue.peek()};
    while ((entityDelete != nullptr)
           && (entityDelete->tickNum <= desiredTick)) {
        if (registry.valid(entityDelete->entity)) {
            world.entityLocator.removeEntity(entityDelete->entity);
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

void EntityLifetimeSystem::processEntityInits(Uint32 desiredTick)
{
    // Immediately process any player entity messages, push the rest into a 
    // secondary queue.
    {
        EntityInit entityInit{};
        while (entityInitQueue.pop(entityInit)) {
            // If the player entity is present, process it and erase it from 
            // the message.
            for (auto it = entityInit.entityData.begin();
                 it != entityInit.entityData.end();) {
                if (it->entity == world.playerEntity) {
                    processEntityData(entityInit.tickNum, *it);
                    it = entityInit.entityData.erase(it);
                }
                else {
                    ++it;
                }
            }

            // If there are NPC entities remaining in the message, push it.
            if (entityInit.entityData.size() > 0) {
                entityInitSecondaryQueue.push(entityInit);
            }
        }
    }

    // Process all messages in the secondary queue.
    while (!(entityInitSecondaryQueue.empty())) {
        EntityInit& entityInit{entityInitSecondaryQueue.front()};

        // If we've reached the desired tick, save the rest of the messages for 
        // later.
        if (entityInit.tickNum > desiredTick) {
            break;
        }

        // Process the message.
        for (const auto& entityData : entityInit.entityData) {
            processEntityData(entityInit.tickNum, entityData);
        }

        entityInitSecondaryQueue.pop();
    }
}

void EntityLifetimeSystem::processEntityData(
    Uint32 tickNum, const EntityInit::EntityData& entityData)
{
    entt::registry& registry{world.registry};

    // Create the entity.
    entt::entity newEntity{registry.create(entityData.entity)};
    if (newEntity != entityData.entity) {
        LOG_FATAL("Created entity doesn't match received entity. "
                  "Created: %u, received: %u",
                  newEntity, entityData.entity);
    }

    // All entities have a position.
    registry.emplace<Position>(newEntity, entityData.position);

    // Add any replicated components that the server sent.
    for (const auto& componentVariant : entityData.components) {
        std::visit([&](const auto& component) {
            using T = std::decay_t<decltype(component)>;
            registry.emplace<T>(newEntity, component);
        }, componentVariant);
    }
    
    // Add any client-only or non-replicated components.
    // Note: Be careful with holding onto references here. If components 
    //       are added to the same group, the ref will be invalidated.

    // Entities with an Input are capable of movement, so we add a 
    // PreviousPosition. They'll also already have a replicated Rotation.
    if (const auto* input{registry.try_get<Input>(newEntity)}) {
        const Position& position{registry.get<Position>(newEntity)};
        registry.emplace<PreviousPosition>(newEntity, position);
    }

    // For entities with an AnimationState, we locally add a Sprite so the 
    // Renderer can use it.
    if (const auto* animationState{
            registry.try_get<AnimationState>(newEntity)}) {
        const Sprite* sprite{
            spriteData.getObjectSpriteSet(animationState->spriteSetID)
                .sprites[animationState->spriteIndex]};
        registry.emplace<Sprite>(newEntity, *sprite);

        // When entities have an AnimationState, the server gives them a 
        // Collision. It isn't replicated, so add it manually.
        const Position& position{registry.get<Position>(newEntity)};
        const Collision& collision{registry.emplace<Collision>(
            newEntity, sprite->modelBounds,
            Transforms::modelToWorldCentered(sprite->modelBounds, position))};

        // Entities with Collision get added to the locator.
        world.entityLocator.setEntityLocation(newEntity, collision.worldBounds);
    }

    // If this is the player entity, add any client components specific to it.
    if (newEntity == world.playerEntity) {
        finishPlayerEntity();
        LOG_INFO("Player entity added: %u. Message tick: %u", newEntity,
                 tickNum);
    }
    else {
        LOG_INFO("Peer entity added: %u. Message tick: %u", newEntity,
                 tickNum);
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

} // End namespace Client
} // End namespace AM
