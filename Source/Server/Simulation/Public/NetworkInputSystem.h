#pragma once

#include "ServerNetworkDefs.h"
#include "QueuedEvents.h"
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
     */
    void processMessageDropEvents();

    /**
     * Defaults the entity's inputs (so they don't run off a cliff) and marks
     * them dirty if their inputs changed.
     */
    void handleDropForEntity(entt::entity entityID);

    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access components. */
    World& world;
    /** Used to access the ClientInput MessageSorter. */
    Network& network;

    EventQueue<ClientMessageDropped> messageDroppedQueue;
};

} // namespace Server
} // namespace AM
