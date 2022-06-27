#include "Simulation.h"
#include "Network.h"
#include "EnttGroups.h"
#include "ISimulationExtension.h"
#include "Log.h"
#include "Timer.h"
#include "Tracy.hpp"

namespace AM
{
namespace Server
{

Simulation::Simulation(Network& inNetwork, SpriteData& inSpriteData)
: network(inNetwork)
, world(inSpriteData)
, currentTick(0)
, extension{nullptr}
, clientConnectionSystem(*this, world, network.getEventDispatcher(), network,
                         inSpriteData)
, tileUpdateSystem(world, network.getEventDispatcher(), network)
, clientAOISystem(*this, world, network)
, inputSystem(*this, world, network.getEventDispatcher(), network)
, movementSystem(world)
, movementUpdateSystem(*this, world, network)
, chunkStreamingSystem(world, network.getEventDispatcher(), network)
{
    // Initialize our entt groups.
    EnttGroups::init(world.registry);

    // Register our current tick pointer with the classes that care.
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Simulation::tick()
{
    ZoneScoped;

    /* Run all systems. */
    // Call the project's pre-everything logic.
    if (extension != nullptr) {
        extension->beforeAll();
    }

    // Process client connections and disconnections.
    clientConnectionSystem.processConnectionEvents();

    // Receive and process tile update requests.
    tileUpdateSystem.updateTiles();

    // Call the project's pre-movement logic.
    if (extension != nullptr) {
        extension->afterMapAndLifetimeUpdates();
    }

    // Receive and process client input messages.
    inputSystem.processInputMessages();

    // Move all of our entities.
    movementSystem.processMovements();

    // Update each client entity's "entities in my AOI" list.
    clientAOISystem.updateAOILists();

    // Send any dirty entity movement state to the clients.
    movementUpdateSystem.sendMovementUpdates();

    // Call the project's post-movement logic.
    if (extension != nullptr) {
        extension->afterMovement();
    }

    // Respond to chunk data requests.
    chunkStreamingSystem.sendChunks();

    currentTick++;
}

Uint32 Simulation::getCurrentTick()
{
    return currentTick;
}

void Simulation::setExtension(std::unique_ptr<ISimulationExtension> inExtension)
{
    extension = std::move(inExtension);
}

} // namespace Server
} // namespace AM
