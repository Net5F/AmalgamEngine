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
, lua{}
, currentTick{0}
, extension{nullptr}
, clientConnectionSystem{*this, world, network, inSpriteData}
, nceLifetimeSystem{world, network, inSpriteData, lua, extension.get()}
, tileUpdateSystem{world, network, extension.get()}
, spriteUpdateSystem{*this, world, network, inSpriteData}
, inputSystem{*this, world, network}
, movementSystem{world}
, clientAOISystem{*this, world, network}
, movementSyncSystem{*this, world, network}
, chunkStreamingSystem{world, network}
, scriptDataSystem{world, network}
, mapSaveSystem{world}
{
    // Initialize our entt groups.
    EnttGroups::init(world.registry);

    // Initialize the lua engine.
    lua.open_libraries(sol::lib::base);

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

    // Create and destroy non-client-controlled entities.
    nceLifetimeSystem.processUpdates();

    // Receive and process tile update requests.
    tileUpdateSystem.updateTiles();

    // Receive and process sprite update requests.
    spriteUpdateSystem.updateSprites();

    // Call the project's pre-movement logic.
    if (extension != nullptr) {
        extension->afterMapAndConnectionUpdates();
    }

    // Send updated tile state to nearby clients.
    tileUpdateSystem.sendTileUpdates();

    // Send updated sprite state to nearby clients.
    spriteUpdateSystem.sendSpriteUpdates();

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

    // Respond to script data requests.
    scriptDataSystem.sendScripts();

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
