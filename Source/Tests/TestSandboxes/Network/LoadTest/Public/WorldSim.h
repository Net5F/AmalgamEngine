#pragma once

#include "Timer.h"
#include "SimDefs.h"
#include "entt/entity/registry.hpp"
#include <SDL_stdinc.h>
#include <atomic>

namespace AM
{
class ClientInput;

namespace Client
{
class Network;
}

namespace LTC
{
/**
 * Represents the simulation for a single simulated client.
 *
 * This is a minimal form of the sim, just maintaining tick timing and sending
 * inputs once in a while.
 */
class WorldSim
{
public:
    WorldSim(Client::Network& inNetwork);

    /**
     * Requests to connect to the game server, waits for a ConnectionResponse.
     */
    void connect();

    /**
     * Processes one tick of the "sim", checking if we need to send inputs or
     * not.
     */
    void tick();

    /** Initialize the iteration timer. */
    void initTimer();

private:
    /**
     * Sends the next input message.
     * The simulated clients currently just move back and forth.
     */
    void sendNextInput();

    /** How long the game should wait for the server to send a connection
        response. */
    static constexpr unsigned int CONNECTION_RESPONSE_WAIT_MS = 1000;

    /** How often to send inputs. */
    static constexpr double INPUT_RATE_S = (1 / 2.0);
    static constexpr unsigned int INPUT_RATE_TICKS
        = SIM_TICKS_PER_SECOND * INPUT_RATE_S;

    Client::Network& network;

    /** The entity ID that we were given by the server. */
    entt::entity clientEntity;

    /** Used to time when we should process an iteration. */
    Timer iterationTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;

    /**
     * The number of the tick that we're currently on.
     * Initialized based on the number that the server tells us it's on.
     */
    std::atomic<Uint32> currentTick;

    /** How many ticks are left until we need to send another input message. */
    unsigned int ticksTillInput;

    /** Tracks which direction this simulated client is moving.
        Used for constructing the next input message. */
    bool isMovingRight;
};

} // End namespace LTC
} // End namespace AM
