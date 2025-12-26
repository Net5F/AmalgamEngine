#include "Simulation.h"
#include "SimulationContext.h"
#include "Network.h"
#include "GraphicData.h"
#include "ItemData.h"
#include "CastableData.h"
#include "ISimulationExtension.h"
#include "ChunkUpdateSystem.h"
#include "TileUpdateSystem.h"
#include "EntityLifetimeSystem.h"
#include "PlayerInputSystem.h"
#include "PlayerMovementSystem.h"
#include "NpcMovementSystem.h"
#include "ItemSystem.h"
#include "InventorySystem.h"
#include "CastSystem.h"
#include "ComponentUpdateSystem.h"
#include "GraphicSystem.h"
#include "AVSystem.h"
#include "CameraSystem.h"
#include "Item.h"
#include "Config.h"
#include "Log.h"
#include "entt/entity/registry.hpp"
#include <memory>
#include <string>

namespace AM
{
namespace Client
{
Simulation::Simulation(const SimulationContext& inSimContext)
: simContext{inSimContext}
, network{inSimContext.network}
, graphicData{inSimContext.graphicData}
, itemData{inSimContext.itemData}
, castableData{inSimContext.castableData}
, world{inSimContext}
, currentTick{0}
, extension{nullptr}
, serverConnectionSystem{inSimContext, currentTick}
{
    // Register our current tick pointer with the classes that care.
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);

    // Initialize our systems to prepare them to run.
    initializeSystems();
}

Simulation::~Simulation() = default;

World& Simulation::getWorld()
{
    return world;
}

const World& Simulation::getWorld() const
{
    return world;
}

Uint32 Simulation::getCurrentTick() const
{
    return currentTick;
}

Uint32 Simulation::getReplicationTick() const
{
    return (currentTick + replicationTickOffset.get());
}

void Simulation::tick()
{
    // If we're online, process any server connection events.
    if (!Config::RUN_OFFLINE) {
        using ConnectionState = ServerConnectionSystem::ConnectionState;

        auto previousConnectionState{
            serverConnectionSystem.getConnectionState()};

        serverConnectionSystem.processConnectionEvents();

        auto currentConnectionState{
            serverConnectionSystem.getConnectionState()};

        // If we just lost connection, re-initialize our systems to prepare
        // them for the next run.
        if ((previousConnectionState == ConnectionState::Connected)
            && (currentConnectionState == ConnectionState::Disconnected)) {
            initializeSystems();
        }

        // If we aren't connected, return early.
        if (serverConnectionSystem.getConnectionState()
            != ConnectionState::Connected) {
            return;
        }
    }

    /* Calculate what tick we should be on. */
    // Increment the tick to the next.
    Uint32 targetTick{currentTick + 1};

    // If we're online, apply any adjustments that we receive from the
    // server.
    if (!Config::RUN_OFFLINE) {
        int adjustment{network.transferTickAdjustment()};
        if (adjustment != 0) {
            targetTick += adjustment;

            // Make sure NPC replication takes the adjustment into account.
            replicationTickOffset.applyAdjustment(adjustment);
        }
    }

    // Process ticks until we match what the server wants.
    // Note: This may cause us to not process any ticks, or to process multiple
    //       ticks.
    while (currentTick < targetTick) {
        /* Run all systems. */
        // Call the project's pre-everything logic.
        extension->beforeAll();

        // Process entities that need to be constructed or destructed.
        entityLifetimeSystem->processUpdates();

        // Process chunk updates from the server.
        chunkUpdateSystem->updateChunks();

        // Process tile updates from the server.
        tileUpdateSystem->updateTiles();

        // Call the project's pre-movement logic.
        extension->afterMapAndConnectionUpdates();

        // Process the held user input state and send change requests to the
        // server.
        // Note: Mouse and momentary inputs are processed through our OS event
        //       handling, prior to this tick.
        playerInputSystem->processHeldInputs();

        // Push the new input state into the player's history.
        playerInputSystem->addCurrentInputsToHistory();

        // Process player movement.
        playerMovementSystem->processMovement();

        // Process NPC movement.
        npcMovementSystem->updateNpcs();

        // Process any waiting item definition updates.
        itemSystem->processItemUpdates();

        // Process any waiting inventory updates.
        inventorySystem->processInventoryUpdates();

        // Process ongoing casts and any waiting cast updates.
        castSystem->processCasts();

        // Call the project's post-sim-update logic.
        extension->afterSimUpdate();

        // Process component updates from the server.
        // Note: We do this last because the state that we receive is the
        //       final state for this tick.
        componentUpdateSystem->processUpdates();

        // Update every entity's graphic state.
        graphicSystem->updateAnimations();

        // Update our local-only AV effects and entities.
        avSystem->updateAVEffectsAndEntities();

        // Move all cameras to their new positions.
        cameraSystem->moveCameras();

        // Call the project's post-everything logic.
        extension->afterAll();

        currentTick++;
    }
}

bool Simulation::handleOSEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_MOUSEMOTION: {
            return false;
        }
        default: {
            // Default to assuming it's a momentary input.
            playerInputSystem->processMomentaryInput(event);
            return true;
        }
    }

    return false;
}

void Simulation::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
    graphicSystem->setExtension(extension);

    // Tell the project to initialize its systems.
    extension->initializeSystems();
}

void Simulation::initializeSystems()
{
    // Initialize our engine systems.
    chunkUpdateSystem = std::make_unique<ChunkUpdateSystem>(simContext);
    tileUpdateSystem = std::make_unique<TileUpdateSystem>(simContext);
    entityLifetimeSystem = std::make_unique<EntityLifetimeSystem>(simContext);
    playerInputSystem = std::make_unique<PlayerInputSystem>(simContext);
    playerMovementSystem = std::make_unique<PlayerMovementSystem>(simContext);
    npcMovementSystem = std::make_unique<NpcMovementSystem>(simContext);
    itemSystem = std::make_unique<ItemSystem>(simContext);
    inventorySystem = std::make_unique<InventorySystem>(simContext);
    castSystem = std::make_unique<CastSystem>(simContext);
    componentUpdateSystem = std::make_unique<ComponentUpdateSystem>(simContext);
    graphicSystem = std::make_unique<GraphicSystem>(simContext);
    avSystem = std::make_unique<AVSystem>(simContext);
    cameraSystem = std::make_unique<CameraSystem>(simContext);

    // If the project extension is already set, re-set it to handle that 
    // initialization.
    if (extension) {
        setExtension(extension);
    }
}

} // namespace Client
} // namespace AM
