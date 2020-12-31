#include "Sim.h"
#include "Network.h"
#include "Log.h"

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

Timer timer2;
double time2[4]{};
double tempTime2[4]{};
int timerCounter2 = 0;
void Sim::tick()
{
    accumulatedTime += iterationTimer.getDeltaSeconds(true);

    /* Process as many game ticks as have accumulated. */
    while (accumulatedTime >= SIM_TICK_TIMESTEP_S) {
        /* Run all systems. */
        timer2.updateSavedTime();
        networkConnectionSystem.processConnectionEvents();
        tempTime2[0] += timer2.getDeltaSeconds(true);

        networkInputSystem.processInputMessages();
        tempTime2[1] += timer2.getDeltaSeconds(true);

        movementSystem.processMovements();
        tempTime2[2] += timer2.getDeltaSeconds(true);

        networkUpdateSystem.sendClientUpdates();
        tempTime2[3] += timer2.getDeltaSeconds(true);

        for (unsigned int i = 0; i < 4; ++i) {
            if (tempTime2[i] > time2[i]) {
                time2[i] = tempTime2[i];
            }
            tempTime2[i] = 0;
        }

        if (timerCounter2 == 150) {
            double total = 0;
            for (unsigned int i = 0; i < 4; ++i) {
                total += time2[i];
            }

            LOG_INFO("Connect: %.6f, Input: %.6f, Move: %.6f, Update: %.6f, Total: %.6f", time2[0],
                time2[1], time2[2], time2[3], total);
            for (unsigned int i = 0; i < 4; ++i) {
                time2[i] = 0;
            }
            timerCounter2 = 0;
        }
        else {
            timerCounter2++;
        }

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
