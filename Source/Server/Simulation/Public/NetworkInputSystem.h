#pragma once

#include "NetworkDefs.h"
#include "QueuedEvents.h"
#include "InputChangeRequest.h"
#include "EventSorter.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;

/**
 * This system is in charge of receiving input messages from clients and
 * applying them to the client's entity.
 */
class NetworkInputSystem
{
public:
    NetworkInputSystem(Simulation& inSim, World& inWorld, Network& inNetwork);

    /**
     * Processes incoming EntityUpdate messages.
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
    Simulation& sim;
    /** Used to access components. */
    World& world;
    /** Used to access the dispatcher. */
    Network& network;

    EventQueue<InputChangeRequest> inputChangeRequestQueue;
    EventSorter<InputChangeRequest> inputChangeRequestSorter;
};

} // namespace Server
} // namespace AM
