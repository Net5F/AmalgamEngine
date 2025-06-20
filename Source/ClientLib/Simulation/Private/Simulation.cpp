#include "Simulation.h"
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
Simulation::Simulation(EventDispatcher& inUiEventDispatcher, Network& inNetwork,
                       GraphicData& inGraphicData, ItemData& inItemData,
                       CastableData& inCastableData)
: network{inNetwork}
, graphicData{inGraphicData}
, itemData{inItemData}
, castableData{inCastableData}
, world{*this, network, inGraphicData, inItemData, inCastableData}
, currentTick{0}
, extension{nullptr}
, serverConnectionSystem{world, inUiEventDispatcher, network, inGraphicData,
                         currentTick}
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
        if (extension != nullptr) {
            extension->beforeAll();
        }

        // Process entities that need to be constructed or destructed.
        entityLifetimeSystem->processUpdates();

        // Process chunk updates from the server.
        chunkUpdateSystem->updateChunks();

        // Process tile updates from the server.
        tileUpdateSystem->updateTiles();

        // Call the project's pre-movement logic.
        if (extension != nullptr) {
            extension->afterMapAndConnectionUpdates();
        }

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
        if (extension != nullptr) {
            extension->afterSimUpdate();
        }

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
        if (extension != nullptr) {
            extension->afterAll();
        }

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

void Simulation::setExtension(std::unique_ptr<ISimulationExtension> inExtension)
{
    extension = std::move(inExtension);
    graphicSystem->setExtension(extension.get());

    if (extension != nullptr) {
        // Tell the project to initialize its systems.
        extension->initializeSystems();
    }
}

entt::sink<entt::sigh<void()>>& Simulation::getSimulationStartedSink()
{
    return serverConnectionSystem.simulationStarted;
}

entt::sink<entt::sigh<void(ConnectionError)>>&
    Simulation::getServerConnectionErrorSink()
{
    return serverConnectionSystem.serverConnectionError;
}

entt::sink<entt::sigh<void(const CastFailed&)>>&
    Simulation::getCastFailedSink()
{
    if (!castSystem) {
        LOG_FATAL("Tried to subscribe to a system signal before the sim was "
                  "fully constructed.");
    }
    return castSystem->castFailed;
}

void Simulation::initializeSystems()
{
    // Initialize our engine systems.
    chunkUpdateSystem = std::make_unique<ChunkUpdateSystem>(world, network);
    tileUpdateSystem = std::make_unique<TileUpdateSystem>(world, network);
    entityLifetimeSystem = std::make_unique<EntityLifetimeSystem>(
        *this, world, network, graphicData);
    playerInputSystem
        = std::make_unique<PlayerInputSystem>(*this, world, network);
    playerMovementSystem
        = std::make_unique<PlayerMovementSystem>(*this, world, network);
    npcMovementSystem
        = std::make_unique<NpcMovementSystem>(*this, world, network);
    itemSystem = std::make_unique<ItemSystem>(world, network, itemData);
    inventorySystem
        = std::make_unique<InventorySystem>(world, network, itemData);
    castSystem = std::make_unique<CastSystem>(*this, network, graphicData,
                                              castableData);
    componentUpdateSystem = std::make_unique<ComponentUpdateSystem>(
        *this, world, network, graphicData);
    graphicSystem = std::make_unique<GraphicSystem>(world, graphicData);
    avSystem = std::make_unique<AVSystem>(world, graphicData);
    cameraSystem = std::make_unique<CameraSystem>(world);

    // Tell the project to initialize its systems.
    if (extension != nullptr) {
        extension->initializeSystems();
    }
}

} // namespace Client
} // namespace AM
