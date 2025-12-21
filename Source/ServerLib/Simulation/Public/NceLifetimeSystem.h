#pragma once

#include "EntityInitRequest.h"
#include "EntityDeleteRequest.h"
#include "QueuedEvents.h"
#include <queue>

namespace AM
{
namespace Server
{
struct SimulationContext;
class World;
class Network;
class ISimulationExtension;

/**
 * Manages creation and destruction of non-client entities.
 *
 * Non-client entities are any entity not controlled by a client (dynamic
 * objects, NPCs, etc).
 */
class NceLifetimeSystem
{
public:
    NceLifetimeSystem(const SimulationContext& inSimContext);

    /**
     * Processes any waiting EntityInitRequest or EntityDelete messages.
     */
    void processUpdateRequests();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * Either creates the given entity and initializes it, or re-creates it
     * and queues an init for next tick.
     */
    void handleInitRequest(const EntityInitRequest& entityInitRequest);

    /**
     * Creates the given entity. If there was an error while running the init
     * script, sends the error to the requesting client.
     */
    void createEntity(const EntityInitRequest& entityInitRequest);

    /**
     * If the given entity is valid, deletes it.
     */
    void handleDeleteRequest(const EntityDeleteRequest& entityDeleteRequest);

    /** Used to add/remove entities. */
    World& world;
    /** Used to send error messages if entity creation fails. */
    Network& network;
    /** Contains the project's simulation extension functions.
        Used for checking if entity creation requests are valid. */
    ISimulationExtension* extension;

    /** Holds entity that need to be re-initialized on the next tick. */
    std::queue<EntityInitRequest> entityReInitQueue;

    EventQueue<EntityInitRequest> entityInitRequestQueue;
    EventQueue<EntityDeleteRequest> entityDeleteRequestQueue;
};

} // End namespace Server
} // End namespace AM
