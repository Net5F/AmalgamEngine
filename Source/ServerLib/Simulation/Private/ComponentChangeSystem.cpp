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
, nameChangeRequestQueue{inNetwork.getEventDispatcher()}
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
    NameChangeRequest nameChangeRequest{};
    while (nameChangeRequestQueue.pop(nameChangeRequest)) {
        // If the project says the request isn't valid, skip it.
        if ((extension != nullptr)
            && !(extension->isNameChangeRequestValid(
                nameChangeRequest))) {
            continue;
        }

        registry.replace<Name>(nameChangeRequest.entity,
                               nameChangeRequest.name);
    }

    AnimationStateChangeRequest animationStateChangeRequest{};
    while (animationStateChangeRequestQueue.pop(animationStateChangeRequest)) {
        // If the project says the request isn't valid, skip it.
        if ((extension != nullptr)
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
    const auto& animationState{registry.get<AnimationState>(entity)};
    auto [position, sprite] = registry.try_get<Position, Sprite>(entity);
    if (position && sprite) {
        const Sprite* newSprite{
            spriteData.getObjectSpriteSet(animationState.spriteSetID)
                .sprites[animationState.spriteIndex]};

        // Note: We assume that an entity with Position and AnimationState
        //       always has a Collision.
        Collision& collision{
            registry.patch<Collision>(entity, [&](Collision& collision) {
                collision.modelBounds = newSprite->modelBounds;
                collision.worldBounds = Transforms::modelToWorldCentered(
                    newSprite->modelBounds, *position);
            })};

        world.entityLocator.setEntityLocation(entity, collision.worldBounds);
    }
}

} // namespace Server
} // namespace AM