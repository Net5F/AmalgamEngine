#include "SimulatedClient.h"

namespace AM
{
namespace LTC
{

SimulatedClient::SimulatedClient()
: worldSim(network)
{
    // Connect to the server.
    worldSim.connect();
}

void SimulatedClient::tick()
{
    // Process the world sim.
    worldSim.tick();

    // Process the network.
    network.tick();
}

} // End namespace LTC
} // End namespace AM
