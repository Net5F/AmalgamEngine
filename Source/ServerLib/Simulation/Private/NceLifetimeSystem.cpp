#include "NceLifetimeSystem.h"
#include "World.h"
#include "Network.h"
#include "Name.h"
#include "Position.h"
#include "InitScript.h"
#include "ISimulationExtension.h"
#include "Log.h"

namespace AM
{
namespace Server
{

NceLifetimeSystem::NceLifetimeSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, extension{nullptr}
, objectReInitQueue{}
, objectInitRequestQueue{inNetwork.getEventDispatcher()}
, deleteQueue{inNetwork.getEventDispatcher()}
{
}

void NceLifetimeSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = std::move(inExtension);
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
        initDynamicObject(entt::null, objectInitRequest);
    }
}

void NceLifetimeSystem::initDynamicObject(entt::entity newEntity,
    const DynamicObjectInitRequest& objectInitRequest)
{
    world.constructDynamicObject(Name{objectInitRequest.name},
                                 objectInitRequest.position,
                                 objectInitRequest.animationState,
                                 InitScript{objectInitRequest.initScript});
}

} // End namespace Server
} // End namespace AM
