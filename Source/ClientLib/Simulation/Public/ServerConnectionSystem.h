#pragma once

#include "QueuedEvents.h"
#include "ConnectionRequest.h"
#include "ConnectionResponse.h"
#include "ConnectionError.h"
#include "WorldLogoutUpdate.h"
#include "DisconnectNotice.h"
#include "Timer.h"
#include "entt/signal/fwd.hpp"
#include <SDL3/SDL_stdinc.h>
#include <queue>
#include <atomic>

namespace AM
{
namespace Client
{
struct SimulationContext;
class Simulation;
class World;
class Network;
class GraphicData;

/**
 * Processes server connect/disconnect events and initializes the sim state
 * when we connect.
 *
 * Also passes the server's WorldLogoutUpdates to the UI. When the server tells
 * us our logout completed, we disconnect.
 * Note: The UI sends WorldLogoutRequests to the server directly.
 */
class ServerConnectionSystem
{
public:
    enum class ConnectionState { Disconnected, AwaitingResponse, Connected };

    ServerConnectionSystem(const SimulationContext& inSimContext,
                           std::atomic<Uint32>& inCurrentTick);

    /**
     * Waits for connection events from the UI. When one is received, attempts
     * to connect to the server.
     *
     * While connected, detects disconnects and sends the disconnect event,
     * and processes logout updates.
     */
    void processConnectionEvents();

    /** Returns the current state of our connection to the server. */
    ConnectionState getConnectionState();

private:
    /** How long the sim should wait for the server to send a connection
        response, in seconds. */
    static constexpr double CONNECTION_RESPONSE_WAIT_S{5};

    void onUIConnectionRequest(ConnectionRequest& connectionRequest);

    /**
     * Passes any received logout updates to the UI. If our logout completed,
     * disconnects and returns to the Disconnected state.
     */
    void processLogoutUpdates();

    /**
     * Disconnects the endpoint, returns to the Disconnected state, and sends
     * a ConnectionError with the given type to the UI.
     */
    void abortConnectionAttempt(ConnectionError::Type errorType);

    /**
     * Sends a ConnectionError with the given type to the UI, disconnects the
     * endpoint, and resets all sim state.
     */
    void disconnectFromServer(ConnectionError::Type errorType);

    /**
     * Requests to connect to the game server, waits for an assigned EntityID,
     * and constructs the player.
     */
    void initSimState(ConnectionResponse& connectionResponse);

    /**
     * Fills in player data as if we connected to the server.
     */
    void initMockSimState();

    /**
     * Clears all sim state. Used when the connection is lost.
     */
    void clearSimState();

    /**
     * Returns the ConnectionError type that corresponds to the given
     * DisconnectNotice reason.
     */
    ConnectionError::Type
        getConnectionErrorType(DisconnectNotice::Reason reason);

    /**
     * Disconnects the endpoint, discards any queued connection events,
     * returns to the Disconnected state, and sends SimulationStopped.
     */
    void closeConnection();

    World& world;
    Network& network;
    GraphicData& graphicData;

    /** Used to send connection events to the UI. */
    entt::dispatcher& simEventDispatcher;

    /** The sim's current tick. Set when we receive a connection response.
        Note: This is the only system that should have a mutable reference. */
    std::atomic<Uint32>& currentTick;

    /** Connection requests, received from the UI. */
    std::queue<ConnectionRequest> connectionRequestQueue;

    /** Connection responses, received from the server. */
    EventQueue<ConnectionResponse> connectionResponseQueue;

    /** Connection error events, received from the server. */
    EventQueue<ConnectionError> connectionErrorQueue;

    /** Logout updates, received from the server. */
    EventQueue<WorldLogoutUpdate> logoutUpdateQueue;

    /** Disconnect notices, received from the server. */
    EventQueue<DisconnectNotice> disconnectNoticeQueue;

    /** Tracks the state of our connection with the server. */
    ConnectionState connectionState;

    /** Times our connection attempt, so we can time out if necessary. */
    Timer connectionAttemptTimer;
};

} // namespace Client
} // namespace AM
