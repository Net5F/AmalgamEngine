#include "NceLifetimeSystem.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "ISimulationExtension.h"
#include "EntityType.h"
#include "Name.h"
#include "Position.h"
#include "Collision.h"
#include "InitScript.h"
#include "Transforms.h"
#include "Log.h"

namespace AM
{
namespace Server
{

NceLifetimeSystem::NceLifetimeSystem(World& inWorld,
                                     Network& inNetwork,
                                     SpriteData& inSpriteData,
                                     const ISimulationExtension* inExtension)
: world{inWorld}
, spriteData{inSpriteData}
, extension{inExtension}
, objectReInitQueue{}
, objectInitRequestQueue{inNetwork.getEventDispatcher()}
, deleteQueue{inNetwork.getEventDispatcher()}
{
}

void NceLifetimeSystem::processUpdates()
{
    // Process any entities that are waiting for re-initialization.
    while (!(objectReInitQueue.empty())) {
        DynamicObjectInitRequest& queuedObjectInit{objectReInitQueue.front()};
        initDynamicObject(queuedObjectInit.entity, queuedObjectInit);
        objectReInitQueue.pop();
    }

    // If we've been requested to create an entity, create it.
    DynamicObjectInitRequest objectCreateRequest{};
    while (objectInitRequestQueue.pop(objectCreateRequest)) {
        createDynamicObject(objectCreateRequest);
    }

    // TODO: Handle EntityDelete
}

void NceLifetimeSystem::createDynamicObject(
    const DynamicObjectInitRequest& objectInitRequest)
{
    // If the project says the area isn't editable, skip this request.
    TilePosition entityTilePos{objectInitRequest.position.asTilePosition()};
    if ((extension != nullptr)
        && !(extension->isExtentEditable(
            {entityTilePos.x, entityTilePos.x, 1, 1}))) {
        return;
    }

    // If the message contains an entity ID, re-initialize the given entity.
    if (objectInitRequest.entity != entt::null) {
        // Double-check that the ID is actually in use.
        if (world.entityIDIsInUse(objectInitRequest.entity)) {
            // This is an existing entity. Remove all of its components. 
            for (auto [id, storage] : world.registry.storage()) {
                storage.remove(objectInitRequest.entity);
            }

            // Remove it from the entity locator.
            world.entityLocator.removeEntity(objectInitRequest.entity);

            // Queue an init for next tick.
            // Note: Since the entity was removed from the locator, AOISystem
            //       will tell nearby clients to delete it. Then, when we re-
            //       init it, AOISystem will send them the new data.
            objectReInitQueue.push(objectInitRequest);

            LOG_INFO("Re-initialized dynamic object with entityID: %u",
                     objectInitRequest.entity);
        }
    }
    else {
        // No ID, create a new entity and initialize it.
        entt::entity newEntity{world.registry.create()};
        initDynamicObject(newEntity, objectInitRequest);

        LOG_INFO("Constructed dynamic object with entityID: %u", newEntity);
    }
}

void NceLifetimeSystem::initDynamicObject(entt::entity newEntity,
    const DynamicObjectInitRequest& objectInitRequest)
{
    entt::registry& registry{world.registry};

    // Construct the standard components.
    // Note: Be careful with holding onto references here. If components 
    //       are added to the same group, the ref will be invalidated.
    registry.emplace<EntityType>(newEntity, EntityType::DynamicObject);
    registry.emplace<Name>(newEntity, Name{objectInitRequest.name});

    registry.emplace<Position>(newEntity, objectInitRequest.position);
    registry.emplace<Rotation>(newEntity, objectInitRequest.rotation);

    const ObjectSpriteSet& spriteSet{
        spriteData.getObjectSpriteSet(objectInitRequest.spriteSetID)};
    registry.emplace<ObjectSpriteSet>(newEntity, spriteSet);

    // Note: The server doesn't have any need for a Sprite component on dynamic
    //       entities, we just get it from the sprite set + rotation.
    const Sprite& sprite{
        *(spriteSet.sprites[registry.get<Rotation>(newEntity).direction])};

    // Note: Every entity needs a Collision for the EntityLocator to use.
    const Collision& collision{registry.emplace<Collision>(
        newEntity, sprite.modelBounds,
        Transforms::modelToWorldCentered(sprite.modelBounds,
                                         registry.get<Position>(newEntity)))};

    registry.emplace<InitScript>(newEntity, objectInitRequest.initScript);

    // TODO: Run the given init script.
    LOG_INFO("%s", objectInitRequest.initScript.c_str());

    // Start tracking the entity in the locator.
    // Note: Since the entity was added to the locator, clients 
    //       will be told by ClientAOISystem to construct it.
    world.entityLocator.setEntityLocation(newEntity,
                                          collision.worldBounds);
}

} // End namespace Server
} // End namespace AM
