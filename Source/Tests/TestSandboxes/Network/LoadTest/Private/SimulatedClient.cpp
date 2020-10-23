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
    network.initTimer();

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

} // End namespace LTC
} // End namespace AM
