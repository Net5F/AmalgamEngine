#include "Simulation.h"
#include "Network.h"
#include "EnttGroups.h"
#include "Log.h"
#include "Timer.h"
#include "Tracy.hpp"

namespace AM
{
namespace Server
{

const char* const FRAME_NAME = "Simulation";

Simulation::Simulation(EventDispatcher& inNetworkEventDispatcher,
                       Network& inNetwork, SpriteData& inSpriteData)
: network(inNetwork)
, world(inSpriteData)
, clientConnectionSystem(*this, world, inNetworkEventDispatcher, network,
                         inSpriteData)
, tileUpdateSystem(world, inNetworkEventDispatcher, network)
, clientAOISystem(*this, world, network)
, inputSystem(*this, world, inNetworkEventDispatcher, network)
, movementSystem(world)
, movementUpdateSystem(*this, world, network)
, chunkStreamingSystem(world, inNetworkEventDispatcher, network)
, currentTick(0)
{
    // Initialize our entt groups.
    EnttGroups::init(world.registry);

    // Register our current tick pointer with the classes that care.
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Simulation::tick()
{
    FrameMarkStart(FRAME_NAME);

    /* Run all systems. */
    // Process client connections and disconnections.
    clientConnectionSystem.processConnectionEvents();

    // Receive and process tile update requests.
    tileUpdateSystem.updateTiles();

    // Receive and process client input messages.
    inputSystem.processInputMessages();

    // Move all of our entities.
    movementSystem.processMovements();

    // Update each client entity's "entities in my AOI" list.
    clientAOISystem.updateAOILists();

    // Send any dirty entity movement state to the clients.
    movementUpdateSystem.sendMovementUpdates();

    // Respond to chunk data requests.
    chunkStreamingSystem.sendChunks();

    currentTick++;

    FrameMarkEnd(FRAME_NAME);
}

Uint32 Simulation::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
