#include "Sim.h"
#include "Network.h"
#include "Log.h"
#include "Profiler.h"

namespace AM
{
namespace Server
{
Sim::Sim(Network& inNetwork)
: world()
, network(inNetwork)
, networkConnectionSystem(*this, world, network)
, networkInputSystem(*this, world, network)
, movementSystem(world)
, networkUpdateSystem(*this, world, network)
, accumulatedTime(0.0)
, currentTick(0)
{
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Sim::tick()
{
    accumulatedTime += iterationTimer.getDeltaSeconds(true);

    /* Process as many game ticks as have accumulated. */
    while (accumulatedTime >= SIM_TICK_TIMESTEP_S) {
        /* Run all systems. */
        BEGIN_CPU_SAMPLE(SimTick);
        networkConnectionSystem.processConnectionEvents();

        networkInputSystem.processInputMessages();

        movementSystem.processMovements();

        networkUpdateSystem.sendClientUpdates();
        END_CPU_SAMPLE();

        /* Prepare for the next tick. */
        accumulatedTime -= SIM_TICK_TIMESTEP_S;
        if (accumulatedTime >= SIM_TICK_TIMESTEP_S) {
            LOG_INFO("Detected a request for multiple game ticks in the same frame. Sim tick was delayed by: %.8fs.",
                     accumulatedTime);
        }
        else if (accumulatedTime >= GAME_DELAYED_TIME_S) {
            // Sim missed its ideal call time, could be our issue or general
            // system slowness.
            LOG_INFO("Detected a delayed sim tick. Sim tick was delayed by: %.8fs.",
                     accumulatedTime);
        }

        // Check our execution time.
        double executionTime = iterationTimer.getDeltaSeconds(false);
        if (executionTime > SIM_TICK_TIMESTEP_S) {
            LOG_INFO("Overran our sim iteration time. executionTime: %.8f",
                     executionTime);
        }

        currentTick++;
    }
}

void Sim::initTimer()
{
    iterationTimer.updateSavedTime();
}

double Sim::getTimeTillNextIteration()
{
    // The time since accumulatedTime was last updated.
    double timeSinceIteration = iterationTimer.getDeltaSeconds(false);
    return (SIM_TICK_TIMESTEP_S - (accumulatedTime + timeSinceIteration));
}

Uint32 Sim::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
