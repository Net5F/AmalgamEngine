#pragma once

#include "EntityInitRequest.h"
#include "EntityDelete.h"
#include "QueuedEvents.h"
#include <queue>

namespace AM
{
namespace Server
{

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
    NceLifetimeSystem(World& inWorld, Network& inNetwork);

    /**
     * Processes any waiting EntityInitRequest or EntityDelete messages.
     */
    void processUpdateRequests();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * Either creates the given object and initializes it, or re-creates it 
     * and queues an init for next tick.
     */
    void createEntity(const EntityInitRequest& entityInitRequest);

    /** Used to add/remove entities. */
    World& world;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if entity creation requests are valid. */
    ISimulationExtension* extension;

    /** Holds entity that need to be re-initialized on the next tick. */
    std::queue<EntityInitRequest> entityReInitQueue;

    EventQueue<EntityInitRequest> entityInitRequestQueue;
    EventQueue<EntityDelete> deleteQueue;
};

} // End namespace Server
} // End namespace AM
