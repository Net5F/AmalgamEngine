#include "Simulation.h"
#include "Network.h"
#include "EnttGroups.h"
#include "Log.h"
#include "Profiler.h"

namespace AM
{
namespace Server
{
Simulation::Simulation(EventDispatcher& inNetworkEventDispatcher,
                       Network& inNetwork, SpriteData& inSpriteData)
: network(inNetwork)
, world(inSpriteData)
, clientConnectionSystem(*this, world, inNetworkEventDispatcher, network, inSpriteData)
, tileUpdateSystem(world, inNetworkEventDispatcher, network)
, clientAOISystem(*this, world, network)
, inputUpdateSystem(*this, world, inNetworkEventDispatcher, network)
, movementSystem(world)
, clientUpdateSystem(*this, world, network)
, chunkStreamingSystem(world, inNetworkEventDispatcher, network)
, currentTick(0)
{
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);

    // Initialize our entt groups.
    EnttGroups::init(world.registry);
}

void Simulation::tick()
{
    /* Run all systems. */
    BEGIN_CPU_SAMPLE(SimulationTick);
    // Process client connections and disconnections.
    clientConnectionSystem.processConnectionEvents();

    // Receive and process tile update requests.
    tileUpdateSystem.updateTiles();

    // Receive and process client input messages.
    inputUpdateSystem.processInputMessages();

    // Move all of our entities.
    movementSystem.processMovements();

    // Update each client entity's "entities in my AOI" list.
    clientAOISystem.updateAOILists();

    // Send any dirty entity movement state to the clients.
    clientUpdateSystem.sendClientUpdates();

    // Respond to chunk data requests.
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
