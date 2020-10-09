#pragma once

#include "World.h"
#include "NetworkConnectionSystem.h"
#include "NetworkInputSystem.h"
#include "MovementSystem.h"
#include "NetworkUpdateSystem.h"
#include "Timer.h"
#include <atomic>

namespace AM
{
namespace Server
{
class Network;

/**
 *
 */
class Game
{
public:
    Game(Network& inNetwork);

    /**
     * Runs an iteration of the game loop.
     */
    void tick();

    /** Initialize the iteration timer. */
    void initTimer();

    double getAccumulatedTime();

    Uint32 getCurrentTick();

private:
    /** An unreasonable amount of time for the game tick to be late by. */
    static constexpr double GAME_DELAYED_TIME_S = .001;

    World world;
    Network& network;

    NetworkConnectionSystem networkConnectionSystem;
    NetworkInputSystem networkInputSystem;
    MovementSystem movementSystem;
    NetworkUpdateSystem networkUpdateSystem;

    /** Used to time when we should process an iteration. */
    Timer iterationTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;

    /**
     * The number of the tick that we're currently on.
     */
    std::atomic<Uint32> currentTick;
};

} // namespace Server
} // namespace AM
