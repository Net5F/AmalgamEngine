#pragma once

#include "NetworkDefs.h"
#include "ServerNetworkDefs.h"
#include "QueuedEvents.h"
#include "entt/fwd.hpp"

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;
class SpriteData;

/**
 * This system is in charge of processing client connect/disconnect events and
 * updating the client's entity.
 */
class ClientConnectionSystem
{
public:
    ClientConnectionSystem(Simulation& inSim, World& inWorld,
                           EventDispatcher& inNetworkEventDispatcher,
                           Network& inNetwork, SpriteData& inSpriteData);

    /** Processes the effects of new connections and disconnects on the sim. */
    void processConnectionEvents();

private:
    /**
     * Processes all newly connected clients, adding them to the sim.
     */
    void processConnectEvents();

    /**
     * Processes all newly disconnected clients, removing them from the sim.
     */
    void processDisconnectEvents();

    /**
     * Sends a connection response to the client with the given networkID.
     *
     * @param networkID  The client's network ID to send the connection response
     *                   to.
     * @param newEntity  The entity that was created for this client.
     */
    void sendConnectionResponse(NetworkID networkID, entt::entity newEntity,
                                float spawnX, float spawnY);

    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access components. */
    World& world;
    /** Used to send connection responses and receive connection events. */
    Network& network;

    /** Used for getting the default sprite's data when constructing client
        entities. */
    SpriteData& spriteData;

    EventQueue<ClientConnected> clientConnectedQueue;
    EventQueue<ClientDisconnected> clientDisconnectedQueue;
};

} // End namespace Server
} // End namespace AM
