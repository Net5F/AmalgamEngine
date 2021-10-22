#include "Simulation.h"
#include "Network.h"
#include "Log.h"
#include "Profiler.h"

namespace AM
{
namespace Server
{
Simulation::Simulation(EventDispatcher& inNetworkEventDispatcher, Network& inNetwork, SpriteData& inSpriteData)
: network(inNetwork)
, world(inSpriteData)
, clientConnectionSystem(*this, world, inNetworkEventDispatcher, network)
, inputUpdateSystem(*this, world, inNetworkEventDispatcher, network)
, movementSystem(world)
, clientUpdateSystem(*this, world, network)
, chunkStreamingSystem(world, inNetworkEventDispatcher, network)
, currentTick(0)
{
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Simulation::tick()
{
    /* Run all systems. */
    BEGIN_CPU_SAMPLE(SimulationTick);
    clientConnectionSystem.processConnectionEvents();

    inputUpdateSystem.processInputMessages();

    movementSystem.processMovements();

    clientUpdateSystem.sendClientUpdates();

    chunkStreamingSystem.sendChunks();
    END_CPU_SAMPLE();

    currentTick++;
}

Uint32 Simulation::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
