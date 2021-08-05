#include "SimulatedClient.h"
#include "SharedConfig.h"

namespace AM
{
namespace LTC
{
SimulatedClient::SimulatedClient()
: networkCaller(std::bind_front(&Client::Network::tick, &network),
                SharedConfig::NETWORK_TICK_TIMESTEP_S, "Network", true)
, worldSim(network)
, simCaller(std::bind_front(&WorldSim::tick, &worldSim),
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
