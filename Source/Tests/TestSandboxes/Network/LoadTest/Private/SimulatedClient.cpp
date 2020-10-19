#include "SimulatedClient.h"

namespace AM
{
namespace LTC
{

void SimulatedClient::tick()
{
    accumulatedTime += iterationTimer.getDeltaSeconds(true);

    // If it's time, send an input.
    while (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
//        if (initialWaitTicks > 0) {
//            initialWaitTicks--;
//        }

        accumulatedTime -= GAME_TICK_TIMESTEP_S;
    }

    // TODO: Receive messages if any are waiting and do nothing with them.
}

} // End namespace LTC
} // End namespace AM
