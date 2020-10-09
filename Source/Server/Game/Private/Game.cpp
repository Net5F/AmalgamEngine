#include "Game.h"
#include "Network.h"
#include "Log.h"

namespace AM
{
namespace Server
{
Game::Game(Network& inNetwork)
: world()
, network(inNetwork)
, networkConnectionSystem(*this, world, network)
, networkInputSystem(*this, world, network)
, movementSystem(world)
, networkUpdateSystem(*this, world, network)
, accumulatedTime(0.0)
, currentTick(0)
{
    world.setSpawnPoint({64, 64});
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Game::tick()
{
    accumulatedTime += iterationTimer.getDeltaSeconds(true);

    /* Process as many game ticks as have accumulated. */
    while (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
        /* Run all systems. */
        networkConnectionSystem.processConnectionEvents();

        networkInputSystem.processInputMessages();

        movementSystem.processMovements();

        networkUpdateSystem.sendClientUpdates();

        /* Prepare for the next tick. */
        accumulatedTime -= GAME_TICK_TIMESTEP_S;
        if (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
            LOG_INFO("Detected a request for multiple game ticks in the same "
                      "frame. Game tick "
                      "must have been massively delayed. Game tick was delayed "
                      "by: %.8fs.",
                      accumulatedTime);
        }
        else if (accumulatedTime >= GAME_DELAYED_TIME_S) {
            // Game missed its ideal call time, could be our issue or general
            // system slowness.
            LOG_INFO("Detected a delayed game tick. Game tick was delayed by: "
                      "%.8fs.",
                      accumulatedTime);
        }

        // Check our execution time.
        double executionTime = iterationTimer.getDeltaSeconds(false);
        if (executionTime > GAME_TICK_TIMESTEP_S) {
            LOG_INFO("Overran our sim iteration time. executionTime: %.8f",
                      executionTime);
        }

        currentTick++;
    }
}

void Game::initTimer()
{
    iterationTimer.updateSavedTime();
}

double Game::getAccumulatedTime()
{
    return accumulatedTime;
}

Uint32 Game::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
