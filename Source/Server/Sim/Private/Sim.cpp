#include "Sim.h"
#include "Network.h"
#include "Log.h"
#include "Profiler.h"

namespace AM
{
namespace Server
{
Sim::Sim(Network& inNetwork)
: world()
, network(inNetwork)
, networkConnectionSystem(*this, world, network)
, networkInputSystem(*this, world, network)
, movementSystem(world)
, networkUpdateSystem(*this, world, network)
, currentTick(0)
{
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Sim::tick()
{
    /* Run all systems. */
    BEGIN_CPU_SAMPLE(SimTick);
    networkConnectionSystem.processConnectionEvents();

    networkInputSystem.processInputMessages();

    movementSystem.processMovements();

    networkUpdateSystem.sendClientUpdates();
    END_CPU_SAMPLE();

    currentTick++;
}

Uint32 Sim::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
