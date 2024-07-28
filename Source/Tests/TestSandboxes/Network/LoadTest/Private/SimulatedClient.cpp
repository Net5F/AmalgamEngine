#include "SimulatedClient.h"
#include "SharedConfig.h"
#include <functional>

namespace AM
{
namespace LTC
{
SimulatedClient::SimulatedClient(unsigned int inInputsPerSecond)
: networkCaller(std::bind_front(&NetworkSimulation::tick, &networkSim),
                SharedConfig::CLIENT_NETWORK_TICK_TIMESTEP_S, "Network", true)
, worldSim(networkSim, inInputsPerSecond)
, simCaller(std::bind_front(&WorldSimulation::tick, &worldSim),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false)
, isConnected(false)
{
}

void SimulatedClient::connect()
{
    // Connect to the server.
    worldSim.connect();

    // Start the tick timer at the current time.
    simCaller.initTimer();
    networkCaller.initTimer();

    isConnected = true;
}

void SimulatedClient::receiveAndProcess()
{
    // Note: This is safe to call, even if connect() is running on another 
    //       thread (it has an internal check).
    networkSim.receiveAndProcess();
}

void SimulatedClient::tick()
{
    // Process the network.
    networkCaller.update();

    // If we're connected, process the world sim.
    if (isConnected) {
        simCaller.update();
    }
}

} // End namespace LTC
} // End namespace AM
