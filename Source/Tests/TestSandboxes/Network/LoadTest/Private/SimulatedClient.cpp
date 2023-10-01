#include "SimulatedClient.h"
#include "SharedConfig.h"

namespace AM
{
namespace LTC
{
SimulatedClient::SimulatedClient(unsigned int inInputsPerSecond)
: network()
, networkCaller(std::bind_front(&Client::Network::tick, &network),
                SharedConfig::CLIENT_NETWORK_TICK_TIMESTEP_S, "Network", true)
, worldSim(network.getEventDispatcher(), network, inInputsPerSecond)
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
