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
            if (entityInit.entity == world.playerEntity) {
                processEntityInit(entityInit);
            }
            else {
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
        processEntityInit(entityInit);

        entityInitSecondaryQueue.pop();
    }
}

void EntityLifetimeSystem::processEntityInit(const EntityInit& entityInit)
{
    entt::registry& registry{world.registry};

    // Create the entity.
    entt::entity newEntity{registry.create(entityInit.entity)};
    if (newEntity != entityInit.entity) {
        LOG_FATAL("Created entity doesn't match received entity. "
                  "Created: %u, received: %u",
                  newEntity, entityInit.entity);
    }

    // Add any replicated components that the server sent.
    for (const auto& componentVariant : entityInit.components) {
        std::visit([&](const auto& component) {
            using T = std::decay_t<decltype(component)>;
            registry.emplace<T>(newEntity, component);
        }, componentVariant);
    }
    
    // Add any client-only or non-replicated components.
    // Note: Be careful with holding onto references here. If components 
    //       are added to the same group, the ref will be invalidated.
    if (const auto* position = registry.try_get<Position>(newEntity)) {
        registry.emplace<PreviousPosition>(newEntity, *position);
    }

    const auto* position{registry.try_get<Position>(newEntity)};
    const auto* animationState{registry.try_get<AnimationState>(newEntity)};
    if ((position != nullptr) && (animationState != nullptr)) {
        const Sprite* sprite{
            spriteData.getObjectSpriteSet(animationState->spriteSetID)
                .sprites[animationState->spriteIndex]};
        registry.emplace<Sprite>(newEntity, *sprite);

        // When entities have a Position and AnimationState, the server gives 
        // them a Collision. It isn't sent, so add it manually.
        registry.emplace<Collision>(
            newEntity, sprite->modelBounds,
            Transforms::modelToWorldCentered(sprite->modelBounds, *position));
    }

    // If this is the player entity, add any client components specific to it.
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

} // End namespace Client
} // End namespace AM
