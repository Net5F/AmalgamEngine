#include "ComponentUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "Collision.h"
#include "Transforms.h"
#include "Log.h"

namespace AM
{
namespace Client
{

ComponentUpdateSystem::ComponentUpdateSystem(Simulation& inSimulation,
                                             World& inWorld, Network& inNetwork,
                                             SpriteData& inSpriteData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, componentUpdateQueue{network.getEventDispatcher()}
{
    world.registry.on_update<AnimationState>()
        .connect<&ComponentUpdateSystem::onAnimationStateUpdated>(this);
}

ComponentUpdateSystem::~ComponentUpdateSystem()
{
    world.registry.on_update<AnimationState>()
        .disconnect<&ComponentUpdateSystem::onAnimationStateUpdated>(this);
}

void ComponentUpdateSystem::processUpdates()
{
    // We want to process updates until we've either processed the desired
    // tick, or have run out of data.
    // Note: If a component update needs to be resolved against local predicted
    //       state, that component update needs to be intercepted before it gets
    //       here. Otherwise, it will be auto-applied over your predicted state.
    Uint32 desiredTick{simulation.getReplicationTick()};

    // Immediately process any player entity messages, push the rest into a
    // secondary queue.
    {
        ComponentUpdate componentUpdate{};
        while (componentUpdateQueue.pop(componentUpdate)) {
            if (componentUpdate.entity == world.playerEntity) {
                processComponentUpdate(componentUpdate);
            }
            else {
                componentUpdateSecondaryQueue.push(componentUpdate);
            }
        }
    }

    // Process all messages in the secondary queue.
    while (!(componentUpdateSecondaryQueue.empty())) {
        ComponentUpdate& componentUpdate{componentUpdateSecondaryQueue.front()};

        // If we've reached the desired tick, save the rest of the messages for
        // later.
        if (componentUpdate.tickNum > desiredTick) {
            break;
        }

        // Process the message.
        processComponentUpdate(componentUpdate);

        componentUpdateSecondaryQueue.pop();
    }
}

void ComponentUpdateSystem::processComponentUpdate(
    const ComponentUpdate& componentUpdate)
{
    entt::registry& registry{world.registry};

    for (const auto& componentVariant : componentUpdate.components) {
        std::visit(
            [&](const auto& component) {
                using T = std::decay_t<decltype(component)>;
                registry.emplace_or_replace<T>(componentUpdate.entity,
                                               component);
            },
            componentVariant);
    }
}

void ComponentUpdateSystem::onAnimationStateUpdated(entt::registry& registry,
                                                    entt::entity entity)
{
    // Since the animation state was updated, we need to update the entity's
    // sprite and collision.
    auto [position, animationState]
        = registry.get<Position, AnimationState>(entity);
    const Sprite* newSprite{
        spriteData.getObjectSpriteSet(animationState.spriteSetID)
            .sprites[animationState.spriteIndex]};
    registry.emplace_or_replace<Sprite>(entity, *newSprite);

    // Note: We assume that an entity with AnimationState always has a
    //       Collision.
    const Collision& collision{
        registry.patch<Collision>(entity, [&](Collision& collision) {
            collision.modelBounds = newSprite->modelBounds;
            collision.worldBounds = Transforms::modelToWorldCentered(
                newSprite->modelBounds, position);
        })};

    world.entityLocator.setEntityLocation(entity, collision.worldBounds);
}

} // namespace Client
} // namespace AM
