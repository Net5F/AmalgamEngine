#pragma once

#include "SharedConfig.h"
#include "QueuedEvents.h"
#include "ConnectionResponse.h"
#include "ConnectionError.h"
#include "entt/entity/registry.hpp"
#include <SDL_stdinc.h>
#include <atomic>

namespace AM
{
struct InputChangeRequest;

namespace LTC
{
class NetworkSimulation;

/**
 * Represents the Simulation for a single simulated client.
 *
 * This is a minimal form of the sim, just maintaining tick timing and sending
 * inputs once in a while.
 */
class WorldSimulation
{
public:
    WorldSimulation(NetworkSimulation& network, unsigned int inInputsPerSecond);

    /**
     * Requests to connect to the game server, waits for a ConnectionResponse.
     */
    void connect();

    /**
     * Processes one tick of the "sim", checking if we need to send inputs or
     * not.
     */
    void tick();

private:
    /**
     * Sends the next input message.
     * The simulated clients currently just move back and forth.
     */
    void sendNextInput();

    /** How long the sim should wait for the server to send a connection
        response, in microseconds. */
    static constexpr int CONNECTION_RESPONSE_WAIT_US{1 * 1000 * 1000};

    NetworkSimulation& network;

    /** Connection responses, received from the server. */
    EventQueue<ConnectionResponse> connectionResponseQueue;

    /** Connection error events, received from the Network. */
    EventQueue<Client::ConnectionError> connectionErrorQueue;

    /** The entity ID that we were given by the server. */
    entt::entity clientEntity;

    /**
     * The number of the tick that we're currently on.
     * Initialized based on the number that the server tells us it's on.
     */
    std::atomic<Uint32> currentTick;

    /** How many times we should send a new input, per second. */
    const unsigned int inputsPerSecond;

    /** How many ticks are left until we need to send another input message. */
    unsigned int ticksTillInput;

    /** Tracks which direction this simulated client is moving.
        Used for constructing the next input message. */
    bool isMovingRight;
};

} // End namespace LTC
} // End namespace AM
