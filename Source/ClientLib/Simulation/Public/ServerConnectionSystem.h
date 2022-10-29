#pragma once

#include "QueuedEvents.h"
#include "ConnectionRequest.h"
#include "ConnectionResponse.h"
#include "ConnectionError.h"
#include "Timer.h"
#include "entt/fwd.hpp"
#include "SDL_stdinc.h"
#include <atomic>

namespace AM
{
namespace Client
{
class World;
class Network;
class SpriteData;

/**
 * This system is in charge of processing server connect/disconnect events and 
 * updating the client's entity.
 */
class ServerConnectionSystem
{
public:
    enum class ConnectionState
    {
        Disconnected,
        AwaitingResponse,
        Connected
    };

    ServerConnectionSystem(World& inWorld, EventDispatcher& inUiEventDispatcher,
                           Network& inNetwork, SpriteData& inSpriteData,
                           std::atomic<Uint32>& inCurrentTick);

    /**
     * Waits for connection events from the UI. When one is received, attempts 
     * to connect to the server.
     * 
     * While connected, detects disconnects and sends the disconnect event.
     */
    void processConnectionEvents();

private:
    /** How long the sim should wait for the server to send a connection
        response, in seconds. */
    static constexpr double CONNECTION_RESPONSE_WAIT_S{5};

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

    World& world;

    Network& network;

    SpriteData& spriteData;

    /** The sim's current tick. Set when we receive a connection response.
        Note: This is the only system that should have a mutable reference. */
    std::atomic<Uint32>& currentTick;

    /** Connection requests, received from the UI. */
    EventQueue<ConnectionRequest> connectionRequestQueue;

    /** Connection responses, received from the server. */
    EventQueue<ConnectionResponse> connectionResponseQueue;

    /** Connection error events, received from the Network. */
    EventQueue<ConnectionError> connectionErrorQueue;

    /** Tracks the state of our connection with the server. */
    ConnectionState connectionState;

    /** Times our connection attempt, so we can time out if necessary. */
    Timer connectionAttemptTimer;
};

} // namespace Client
} // namespace AM
