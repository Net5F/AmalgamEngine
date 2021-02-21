#include "SimulatedClient.h"

namespace AM
{
namespace LTC
{
SimulatedClient::SimulatedClient()
: worldSim(network)
, isConnected(false)
{
}

void SimulatedClient::connect()
{
    // Connect to the server.
    worldSim.connect();

    // Start the tick timer at the current time.
    worldSim.initTimer();
//    network.initTimer();

    isConnected = true;
}

void SimulatedClient::tick()
{
    if (isConnected) {
        // Process the world sim.
        worldSim.tick();

        // Process the network.
        network.tick();
    }
}

void SimulatedClient::setNetstatsLoggingEnabled(bool inNetstatsLoggingEnabled)
{
    network.setNetstatsLoggingEnabled(inNetstatsLoggingEnabled);
}

} // End namespace LTC
} // End namespace AM
