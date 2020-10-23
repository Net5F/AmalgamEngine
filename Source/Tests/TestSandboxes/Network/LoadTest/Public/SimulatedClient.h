#pragma once

#include "Network.h"
#include "WorldSim.h"
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
    /**
     * Inits the object and calls worldSim.connect().
     */
    SimulatedClient();

    /**
     * Calls the sim and network ticks.
     */
    void tick();

private:
    Client::Network network;

    WorldSim worldSim;
};

} // End namespace LTC
} // End namespace AM
