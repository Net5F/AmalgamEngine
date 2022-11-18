#pragma once

#include "NetworkDefs.h"
#include "QueuedEvents.h"
#include "InputChangeRequest.h"
#include "EventSorter.h"

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;

/**
 * Receives input messages from clients and applies them to the client's
 * entity.
 */
class InputSystem
{
public:
    InputSystem(Simulation& inSimulation, World& inWorld,
                EventDispatcher& inNetworkEventDispatcher);

    /**
     * Processes incoming InputChangeRequest messages.
     */
    void processInputMessages();

private:
    /**
     * Processes message drop events, which occur when the server received a
     * client's input message late and had to drop it.
     *
     * We default the client's inputs (so they don't run off a cliff) and set
     * a flag so the NetworkUpdateSystem knows that a drop occurred.
     *
     * @param clientID  The ID of the client that we had to drop a message from.
     */
    void handleDroppedMessage(NetworkID clientID);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;

    EventQueue<InputChangeRequest> inputChangeRequestQueue;
    EventSorter<InputChangeRequest> inputChangeRequestSorter;
};

} // namespace Server
} // namespace AM
