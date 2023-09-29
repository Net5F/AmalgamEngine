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
, entityReInitQueue{}
, entityInitRequestQueue{inNetwork.getEventDispatcher()}
, entityDeleteRequestQueue{inNetwork.getEventDispatcher()}
{
}

void NceLifetimeSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = std::move(inExtension);
}

void NceLifetimeSystem::processUpdateRequests()
{
    // Process any entities that are waiting for re-initialization.
    while (!(entityReInitQueue.empty())) {
        EntityInitRequest& queuedEntityInit{entityReInitQueue.front()};
        world.constructEntity(
            queuedEntityInit.position, queuedEntityInit.components,
            InitScript{queuedEntityInit.initScript}, queuedEntityInit.entity);
        entityReInitQueue.pop();
    }

    // If we've been requested to create an entity, create it.
    EntityInitRequest entityCreateRequest{};
    while (entityInitRequestQueue.pop(entityCreateRequest)) {
        createEntity(entityCreateRequest);
    }

    // If we've been requested to delete an entity, delete it.
    EntityDeleteRequest entityDeleteRequest{};
    while (entityDeleteRequestQueue.pop(entityDeleteRequest)) {
        deleteEntity(entityDeleteRequest);
    }
}

void NceLifetimeSystem::createEntity(
    const EntityInitRequest& entityInitRequest)
{
    // If the project says the request isn't valid, skip it.
    if ((extension != nullptr)
        && !(extension->isEntityInitRequestValid(entityInitRequest))) {
        return;
    }

    // If the message contains an entity ID, re-initialize the given entity.
    if (entityInitRequest.entity != entt::null) {
        // Double-check that the ID is actually in use.
        if (world.entityIDIsInUse(entityInitRequest.entity)) {
            // This is an existing entity. Remove all of its components. 
            for (auto [id, storage] : world.registry.storage()) {
                storage.remove(entityInitRequest.entity);
            }

            // Remove it from the entity locator.
            world.entityLocator.removeEntity(entityInitRequest.entity);

            // Queue an init for next tick.
            // Note: Since the entity was removed from the locator, AOISystem
            //       will tell nearby clients to delete it. Then, when we re-
            //       init it, AOISystem will send them the new data.
            entityReInitQueue.push(entityInitRequest);
        }
    }
    else {
        // No ID, create a new entity and initialize it.
        world.constructEntity(
            entityInitRequest.position, entityInitRequest.components,
            InitScript{entityInitRequest.initScript}, entityInitRequest.entity);
    }
}

void NceLifetimeSystem::deleteEntity(
    const EntityDeleteRequest& entityDeleteRequest)
{
    // If the entity isn't valid or is a client, skip it.
    entt::entity entity{entityDeleteRequest.entity};
    if (!(world.entityIDIsInUse(entity))
        || world.registry.all_of<IsClientEntity>(entity)) {
        return;
    }
    // If the project says the request isn't valid, skip it.
    else if ((extension != nullptr)
             && !(extension->isEntityDeleteRequestValid(entityDeleteRequest))) {
        return;
    }

    world.registry.destroy(entity);
}

} // End namespace Server
} // End namespace AM
