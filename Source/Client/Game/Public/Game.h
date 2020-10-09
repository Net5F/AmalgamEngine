#pragma once

#include "NpcMovementSystem.h"
#include "PlayerMovementSystem.h"
#include "World.h"
#include "PlayerInputSystem.h"
#include "NetworkUpdateSystem.h"
#include "Timer.h"
#include <atomic>

namespace AM
{
namespace Client
{
class Network;

/**
 *
 */
class Game
{
public:
    Game(Network& inNetwork, const std::shared_ptr<SDL2pp::Texture>& inSprites);

    /**
     * Requests to connect to the game server, waits for an assigned EntityID,
     * and constructs the player.
     */
    void connect();

    /**
     * Fills in player data as if we connected to the server.
     */
    void fakeConnection();

    /**
     * Runs an iteration of the game loop.
     */
    void tick();

    /**
     * Processes all waiting user input events, passing any relevant ones to the
     * playerInputSystem.
     */
    void processUserInputEvents();

    /** Initialize the iteration timer. */
    void initTimer();

    World& getWorld();

    /**
     * Returns how far we are temporally into our wait for the next iteration
     * tick. e.g. .01 if we're 10% of the way to the next tick.
     */
    double getIterationProgress();

    Uint32 getCurrentTick();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    /** How long the game should wait for the server to send a connection
     * response. */
    static constexpr unsigned int CONNECTION_RESPONSE_WAIT_MS = 1000;

    /** An unreasonable amount of time for the game tick to be late by. */
    static constexpr double GAME_DELAYED_TIME_S = .001;

    World world;
    Network& network;

    PlayerInputSystem playerInputSystem;
    NetworkUpdateSystem networkUpdateSystem;
    PlayerMovementSystem playerMovementSystem;
    NpcMovementSystem npcMovementSystem;

    /** Used to time when we should process an iteration. */
    Timer iterationTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;

    /**
     * The number of the tick that we're currently on.
     * Initialized based on the number that the server tells us it's on.
     */
    std::atomic<Uint32> currentTick;

    // Temporary until a resource manager is created.
    const std::shared_ptr<SDL2pp::Texture>& sprites;

    /**
     * Turn false to signal that the main loop should end.
     * The game processes the inputs, so it gets to be in charge of program
     * lifespan.
     */
    std::atomic<bool> exitRequested;
};

} // namespace Client
} // namespace AM
