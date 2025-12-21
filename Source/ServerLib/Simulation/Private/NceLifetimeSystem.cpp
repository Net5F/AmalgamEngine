#include "NceLifetimeSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "Database.h"
#include "Network.h"
#include "Name.h"
#include "Position.h"
#include "EntityInitScript.h"
#include "IsClientEntity.h"
#include "Collision.h"
#include "SystemMessage.h"
#include "ISimulationExtension.h"
#include "Log.h"

namespace AM
{
namespace Server
{

NceLifetimeSystem::NceLifetimeSystem(const SimulationContext& inSimContext)
: world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, extension{nullptr}
, entityReInitQueue{}
, entityInitRequestQueue{inSimContext.networkEventDispatcher}
, entityDeleteRequestQueue{inSimContext.networkEventDispatcher}
{
}

void NceLifetimeSystem::processUpdateRequests()
{
    // Process any entities that are waiting for re-initialization.
    while (!(entityReInitQueue.empty())) {
        EntityInitRequest& queuedEntityInit{entityReInitQueue.front()};
        createEntity(queuedEntityInit);
        entityReInitQueue.pop();
    }

    // If we've been requested to create an entity, create it.
    EntityInitRequest entityCreateRequest{};
    while (entityInitRequestQueue.pop(entityCreateRequest)) {
        handleInitRequest(entityCreateRequest);
    }

    // If we've been requested to delete an entity, delete it.
    EntityDeleteRequest entityDeleteRequest{};
    while (entityDeleteRequestQueue.pop(entityDeleteRequest)) {
        handleDeleteRequest(entityDeleteRequest);
    }
}

void NceLifetimeSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void NceLifetimeSystem::handleInitRequest(
    const EntityInitRequest& entityInitRequest)
{
    // If the project says the request isn't valid, do nothing.
    if (!(extension->isEntityInitRequestValid(entityInitRequest))) {
        return;
    }

    // If the message contains a valid entity ID, re-initialize the given entity.
    if (world.registry.valid(entityInitRequest.entity)) {
        // If the requested entity is a client entity, do nothing (we don't 
        // allow clients to re-init client entities).
        if (world.registry.all_of<IsClientEntity>(entityInitRequest.entity)) {
            return;
        }

        // Destroy the entity.
        // Note: This will cause it to be removed from the entity locator, 
        //       triggering ClientAOISystem to tell peers to delete it.
        //       Then, when we re-init it, ClientAOISystem will send them 
        //       the new data. This ensures that we don't leave any old 
        //       components.
        world.registry.destroy(entityInitRequest.entity);

        // Queue an init for next tick.
        entityReInitQueue.push(entityInitRequest);
    }
    else {
        // No ID, create a new entity and initialize it.
        createEntity(entityInitRequest);
    }
}

void NceLifetimeSystem::handleDeleteRequest(
    const EntityDeleteRequest& entityDeleteRequest)
{
    // If the entity isn't valid or is a client, skip it.
    entt::entity entity{entityDeleteRequest.entity};
    if (!(world.registry.valid(entity))
        || world.registry.all_of<IsClientEntity>(entity)) {
        return;
    }
    // If the project says the request isn't valid, skip it.
    else if (!(extension->isEntityDeleteRequestValid(entityDeleteRequest))) {
        return;
    }

    world.registry.destroy(entity);
}

void NceLifetimeSystem::createEntity(const EntityInitRequest& entityInitRequest)
{
    // Create the entity.
    entt::entity newEntity{world.createEntity(entityInitRequest.position,
                                              entityInitRequest.entity)};
    world.registry.emplace<Name>(newEntity, entityInitRequest.name);
    world.registry.emplace<Rotation>(newEntity, entityInitRequest.rotation);

    // Add the graphic components.
    world.addGraphicsComponents(newEntity, entityInitRequest.graphicState);

    // Run the init script. If there was an error, tell the user.
    std::string resultString{
        world.runEntityInitScript(newEntity, entityInitRequest.initScript)};
    if (!(resultString.empty())) {
        network.serializeAndSend(entityInitRequest.netID,
                                 SystemMessage{resultString});
    }
}

} // End namespace Server
} // End namespace AM
