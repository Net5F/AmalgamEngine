#include "NceLifetimeSystem.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "ISimulationExtension.h"
#include "EntityType.h"
#include "Name.h"
#include "Position.h"
#include "Collision.h"
#include "Transforms.h"
#include "Log.h"

namespace AM
{
namespace Server
{

NceLifetimeSystem::NceLifetimeSystem(World& inWorld,
                                     EventDispatcher& inNetworkEventDispatcher,
                                     Network& inNetwork,
                                     SpriteData& inSpriteData,
                                     const ISimulationExtension* inExtension)
: world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, extension{inExtension}
, objectCreateRequestQueue{inNetworkEventDispatcher}
, deleteQueue{inNetworkEventDispatcher}
{
}

void NceLifetimeSystem::processUpdates()
{
    // If we've been requested to create an entity, create it.
    DynamicObjectCreateRequest objectCreateRequest{};
    while (objectCreateRequestQueue.pop(objectCreateRequest)) {
        createDynamicObject(objectCreateRequest);
    }
}

void NceLifetimeSystem::createDynamicObject(
    const DynamicObjectCreateRequest& objectCreateRequest)
{
    // If the project says the area isn't editable, skip this request.
    TilePosition entityTilePos{objectCreateRequest.position.asTilePosition()};
    if ((extension != nullptr)
        && !(extension->isExtentEditable(
            {entityTilePos.x, entityTilePos.x, 1, 1}))) {
        return;
    }

    entt::entity newEntity{world.registry.create()};

    world.registry.emplace<EntityType>(newEntity, EntityType::DynamicObject);
    world.registry.emplace<Name>(newEntity, Name{objectCreateRequest.name});

    const Position& position{world.registry.emplace<Position>(
        newEntity, objectCreateRequest.position)};
    const Rotation& rotation{world.registry.emplace<Rotation>(
        newEntity, objectCreateRequest.rotation)};

    const ObjectSpriteSet& spriteSet{
        spriteData.getObjectSpriteSet(objectCreateRequest.spriteSetID)};
    world.registry.emplace<ObjectSpriteSet>(newEntity, spriteSet);
    const Sprite& sprite{*(spriteSet.sprites[rotation.direction])};

    // Note: Every entity needs a Collision for the EntityLocator to use.
    const Collision& collision{world.registry.emplace<Collision>(
        newEntity, sprite.modelBounds,
        Transforms::modelToWorldCentered(sprite.modelBounds,
                                         position))};

    // Start tracking the entity in the locator.
    // Note: Since the entity was added to the locator, clients 
    //       will be told by ClientAOISystem to construct it.
    world.entityLocator.setEntityLocation(newEntity,
                                          collision.worldBounds);

    LOG_INFO("Constructed dynamic object with entityID: %u", newEntity);
}

} // End namespace Server
} // End namespace AM
