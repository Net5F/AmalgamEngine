#pragma once

#include "NetworkDefs.h"
#include "ClientConnectionEvent.h"
#include "QueuedEvents.h"
#include "entt/fwd.hpp"

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;
class GraphicData;

/**
 * This system is in charge of processing client connect/disconnect events and
 * updating the client's entity.
 */
class ClientConnectionSystem
{
public:
    ClientConnectionSystem(Simulation& inSimulation, World& inWorld,
                           Network& inNetwork, GraphicData& inGraphicData);

    /**
     * Processes new connections and disconnections, updating the sim state
     * appropriately.
     */
    void processConnectionEvents();

private:
    /**
     * Processes all newly connected clients, adding them to the sim.
     */
    void processConnectEvent(const ClientConnected& clientConnected);

    /**
     * Processes all newly disconnected clients, removing them from the sim.
     */
    void processDisconnectEvent(const ClientDisconnected& clientDisconnected);

    /**
     * Sends a connection response to the client with the given networkID.
     *
     * @param networkID  The client's network ID to send the connection response
     *                   to.
     * @param newEntity  The entity that was created for this client.
     */
    void sendConnectionResponse(NetworkID networkID, entt::entity newEntity);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to send connection responses and receive connection events. */
    Network& network;

    /** Used for getting the default graphic's data when constructing client
        entities. */
    GraphicData& graphicData;

    EventQueue<ClientConnectionEvent> clientConnectionEventQueue;
};

} // End namespace Server
} // End namespace AM
