#include "ComponentChangeSystem.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "Collision.h"
#include "ClientSimData.h"
#include "ISimulationExtension.h"
#include "Transforms.h"
#include "Log.h"
#include "tracy/Tracy.hpp"

namespace AM
{
namespace Server
{
ComponentChangeSystem::ComponentChangeSystem(World& inWorld, Network& inNetwork,
                                             SpriteData& inSpriteData)
: world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, extension{nullptr}
, entityNameChangeRequestQueue{inNetwork.getEventDispatcher()}
, animationStateChangeRequestQueue{inNetwork.getEventDispatcher()}
{
    world.registry.on_update<AnimationState>()
        .connect<&ComponentChangeSystem::onAnimationStateUpdated>(this);
}

void ComponentChangeSystem::processChangeRequests()
{
    ZoneScoped;

    entt::registry& registry{world.registry};

    // Process any waiting update requests.
    EntityNameChangeRequest nameChangeRequest{};
    while (entityNameChangeRequestQueue.pop(nameChangeRequest)) {
        // If the entity isn't valid, skip it.
        if (!(world.entityIDIsInUse(nameChangeRequest.entity))) {
            continue;
        }
        // If the project says the request isn't valid, skip it.
        else if ((extension != nullptr)
                 && !(extension->isEntityNameChangeRequestValid(
                     nameChangeRequest))) {
            continue;
        }

        registry.replace<Name>(nameChangeRequest.entity,
                               nameChangeRequest.name);
    }

    AnimationStateChangeRequest animationStateChangeRequest{};
    while (animationStateChangeRequestQueue.pop(animationStateChangeRequest)) {
        // If the entity isn't valid, skip it.
        if (!(world.entityIDIsInUse(animationStateChangeRequest.entity))) {
            continue;
        }
        // If the project says the request isn't valid, skip it.
        else if ((extension != nullptr)
                 && !(extension->isAnimationStateChangeRequestValid(
                     animationStateChangeRequest))) {
            continue;
        }

        registry.replace<AnimationState>(
            animationStateChangeRequest.entity,
            animationStateChangeRequest.animationState);
    }
}

void ComponentChangeSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void ComponentChangeSystem::onAnimationStateUpdated(entt::registry& registry,
                                                    entt::entity entity)
{
    // Since the animation state was updated, we need to update the entity's
    // collision.
    auto [position, animationState]
        = registry.get<Position, AnimationState>(entity);
    const Sprite* newSprite{
        spriteData.getObjectSpriteSet(animationState.spriteSetID)
            .sprites[animationState.spriteIndex]};

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

} // namespace Server
} // namespace AM