#include "Simulation.h"
#include "Network.h"
#include "Log.h"
#include "Profiler.h"

namespace AM
{
namespace Server
{
Simulation::Simulation(Network& inNetwork, SpriteData& inSpriteData)
: world(inSpriteData)
, network(inNetwork)
, networkConnectionSystem(*this, world, network)
, networkInputSystem(*this, world, network)
, movementSystem(world)
, networkUpdateSystem(*this, world, network)
, chunkStreamingSystem(world, network)
, currentTick(0)
{
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Simulation::tick()
{
    /* Run all systems. */
    BEGIN_CPU_SAMPLE(SimulationTick);
    networkConnectionSystem.processConnectionEvents();

    networkInputSystem.processInputMessages();

    movementSystem.processMovements();

    networkUpdateSystem.sendClientUpdates();

//    chunkStreamingSystem.sendChunks();
    END_CPU_SAMPLE();

    currentTick++;
}

Uint32 Simulation::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
