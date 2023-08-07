#include "NceLifetimeSystem.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "Position.h"
#include "Collision.h"
#include "Transforms.h"
#include "Log.h"

namespace AM
{
namespace Server
{

NceLifetimeSystem::NceLifetimeSystem(
    World& inWorld, EventDispatcher& inNetworkEventDispatcher,
    Network& inNetwork, SpriteData& inSpriteData)
: world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, createRequestQueue{inNetworkEventDispatcher}
, deleteQueue{inNetworkEventDispatcher}
{
}

void NceLifetimeSystem::processUpdates()
{
    // TODO: Add position check (like tile updates)
    // If we've been requested to create an entity, create it.
    EntityCreateRequest createRequest{};
    while (createRequestQueue.pop(createRequest)) {
        entt::entity newEntity{world.registry.create()};
        const Position& newPosition{world.registry.emplace<Position>(
            newEntity, createRequest.position)};

        const Sprite& newSprite{world.registry.emplace<Sprite>(
            newEntity, spriteData.getSprite(createRequest.numericID))};

        // Note: Every entity needs a Collision for the EntityLocator to use.
        const Collision& newCollision{world.registry.emplace<Collision>(
            newEntity, newSprite.modelBounds,
            Transforms::modelToWorldCentered(newSprite.modelBounds,
                                             newPosition))};

        // Start tracking the entity in the locator.
        // Note: Since the entity was added to the locator, clients 
        //       will be told by ClientAOISystem to construct it.
        world.entityLocator.setEntityLocation(newEntity,
                                              newCollision.worldBounds);
    }
}

} // End namespace Server
} // End namespace AM
