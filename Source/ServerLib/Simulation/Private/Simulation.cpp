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
: network{inNetwork}
, world{inSpriteData}
, currentTick{0}
, extension{nullptr}
, clientConnectionSystem{*this, world, network.getEventDispatcher(), network,
                         inSpriteData}
, tileUpdateSystem{world, network.getEventDispatcher(), network, extension}
, clientAOISystem{*this, world, network}
, inputSystem{*this, world, network.getEventDispatcher()}
, movementSystem{world}
, movementSyncSystem{*this, world, network}
, chunkStreamingSystem{world, network.getEventDispatcher(), network}
, mapSaveSystem{world}
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
        extension->afterMapAndConnectionUpdates();
    }

    // Send updated tile state to nearby clients.
    tileUpdateSystem.sendTileUpdates();

    // Receive and process client input messages.
    inputSystem.processInputMessages();

    // Move all of our entities.
    movementSystem.processMovements();

    // Call the project's post-movement logic.
    if (extension != nullptr) {
        extension->afterMovement();
    }

    // Update each client entity's "entities in my AOI" list.
    clientAOISystem.updateAOILists();

    // Synchronize entity movement state with the clients.
    movementSyncSystem.sendMovementUpdates();

    // Call the project's post-movement-sync logic.
    if (extension != nullptr) {
        extension->afterMovementSync();
    }

    // Respond to chunk data requests.
    chunkStreamingSystem.sendChunks();

    // If enough time has passed, save the world's tile map state.
    mapSaveSystem.saveMapIfNecessary();

    currentTick++;
}

World& Simulation::getWorld()
{
    return world;
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
