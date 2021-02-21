#include "SimulatedClient.h"

namespace AM
{
namespace LTC
{
SimulatedClient::SimulatedClient()
: networkCaller(std::bind(&Client::Network::tick, &network), NETWORK_TICK_TIMESTEP_S,
      "Network", true)
, worldSim(network)
, simCaller(std::bind(&WorldSim::tick, &worldSim), SIM_TICK_TIMESTEP_S, "Sim", false)
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

void SimulatedClient::tick()
{
    if (isConnected) {
        // Process the world sim.
        simCaller.update();

        // Process the network.
        networkCaller.update();
    }
}

void SimulatedClient::setNetstatsLoggingEnabled(bool inNetstatsLoggingEnabled)
{
    network.setNetstatsLoggingEnabled(inNetstatsLoggingEnabled);
}

} // End namespace LTC
} // End namespace AM
