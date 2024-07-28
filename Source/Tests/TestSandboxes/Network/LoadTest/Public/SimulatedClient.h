#pragma once

#include "NetworkSimulation.h"
#include "WorldSimulation.h"
#include "PeriodicCaller.h"
#include <atomic>

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
    SimulatedClient(unsigned int inInputsPerSecond);

    /**
     * Calls worldSim.connect().
     */
    void connect();

    /**
     * Calls networkSim.receiveAndProcess().
     */
    void receiveAndProcess();

    /**
     * Calls the sim and network ticks.
     */
    void tick();

private:
    NetworkSimulation networkSim;
    PeriodicCaller networkCaller;

    WorldSimulation worldSim;
    PeriodicCaller simCaller;

    /** If true, this client is connected to the server and we've processed the 
        ConnectionResponse. */
    std::atomic<bool> isConnected;
};

} // End namespace LTC
} // End namespace AM
