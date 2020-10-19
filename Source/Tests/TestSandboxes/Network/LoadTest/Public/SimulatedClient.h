#pragma once

#include "Network.h"
#include <SDL_stdinc.h>

namespace AM
{
namespace LTC
{

/**
 * Represents a single simulated client.
 * Maintains only as much state as is necessary to keep the connection going
 * and send inputs.
 */
class SimulatedClient
{
public:
    // TODO: Make a sim class and move this stuff into it.
    /** Two movements per second. */
    static constexpr double TICK_TIMESTEP_S = (1 / 2.0);

    /**
     * Calls the sim and network ticks.
     */
    void tick();

private:
    Client::Network network;

    Timer iterationTimer;
    Uint32 currentTick = 0;
    double accumulatedTime = 0;
};

} // End namespace LTC
} // End namespace AM
