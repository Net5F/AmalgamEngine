#pragma once

#include "NetworkDefs.h"
#include "ClientConnectionEvent.h"
#include "QueuedEvents.h"
#include "WorldLogoutRequest.h"
#include "EnttObserver.h"
#include "entt/fwd.hpp"
#include <unordered_map>

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class Simulation;
class World;
class Network;
class GraphicData;

/**
 * This system is in charge of processing client connect/disconnect events and
 * updating the client's entity.
 *
 * When a client disconnects, its entity lingers in the world for
 * Config::LOGOUT_DELAY_S before being removed. If the same account
 * connects during that time, the new client takes control of the existing
 * entity.
 *
 * Clients can also request an explicit logout. This starts a countdown of
 * Config::LOGOUT_DELAY_S, which is canceled if the entity moves or casts.
 * When the countdown finishes, the entity is removed immediately (it doesn't
 * linger), and the client is told to disconnect.
 *
 * Note: If an account connects while it already has a connected client, the
 *       Network disconnects the old client before telling us about the new
 *       one, so we see it as a normal disconnect followed by a quick reconnect.
 */
class ClientConnectionSystem
{
public:
    ClientConnectionSystem(const SimulationContext& inSimContext);

    /**
     * Processes new connections and disconnections, updating the sim state
     * appropriately. Processes logout requests and updates ongoing logouts.
     * Removes the entities of any clients whose linger time has expired.
     */
    void processConnectionEvents();

private:
    /**
     * Processes a newly connected client. If its account's entity is still
     * lingering, attaches the client to it. Otherwise, creates a new entity.
     */
    void processConnectEvent(const ClientConnected& clientConnected);

    /**
     * Processes a newly disconnected client, starting its entity's linger
     * time. If the client logged out, removes its entity immediately.
     */
    void processDisconnectEvent(const ClientDisconnected& clientDisconnected);

    /**
     * Creates a new entity for the given client.
     */
    entt::entity createClientEntity(const ClientConnected& clientConnected);

    /**
     * Attaches the given client to the given existing entity, whose client
     * disconnected.
     */
    void attachClientToEntity(const ClientConnected& clientConnected,
                              entt::entity entity);

    /**
     * Starts or cancels the requesting client's logout.
     */
    void processLogoutRequest(const WorldLogoutRequest& logoutRequest);

    /**
     * Cancels any logouts whose entity moved or cast, and completes any
     * logouts whose countdown has finished.
     */
    void updateLogouts();

    /**
     * Ends the given account's logout countdown and tells its client.
     */
    void cancelLogout(Sint64 accountID, entt::entity entity);

    /**
     * Tells the given account's client that the logout completed, and
     * disconnects it. The entity is removed once the disconnect event
     * arrives.
     */
    void completeLogout(Sint64 accountID, entt::entity entity);

    /**
     * Returns true if the given entity is moving or trying to move.
     */
    bool entityIsMoving(entt::entity entity);

    /**
     * Removes any entities whose linger time has expired.
     */
    void removeExpiredEntities();

    /**
     * Removes the given account's entity from the sim.
     */
    void removeClientEntity(Sint64 accountID);

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

    EventQueue<WorldLogoutRequest> logoutRequestQueue;

    /** Observes CastState construction, so we can cancel logouts when the
        entity casts something (even instant casts, which never persist a
        CastState across ticks). */
    EnttObserver castStartedObserver;

    /** Maps account IDs to their client entity. Includes entities that are
        lingering. */
    std::unordered_map<Sint64, entt::entity> accountEntityMap;

    /** Maps the account IDs of entities that are lingering -> the tick
        that their entity should be removed on. */
    std::unordered_map<Sint64, Uint32> removalTickMap;

    /** Maps the account IDs of entities that are logging out -> the tick
        that their logout will complete on. */
    std::unordered_map<Sint64, Uint32> logoutTickMap;
};

} // End namespace WorldServer
} // End namespace AM
