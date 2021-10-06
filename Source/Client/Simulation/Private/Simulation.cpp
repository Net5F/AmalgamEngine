#include "Simulation.h"
#include "Network.h"
#include "SpriteData.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "Sprite.h"
#include "Camera.h"
#include "PlayerState.h"
#include "Name.h"
#include "ScreenRect.h"
#include "Paths.h"
#include "Config.h"
#include "SharedConfig.h"
#include "Profiler.h"
#include "Log.h"
#include "entt/entity/registry.hpp"
#include <memory>
#include <string>

namespace AM
{
namespace Client
{
Simulation::Simulation(Network& inNetwork, SpriteData& inSpriteData)
: world(inSpriteData)
, network{inNetwork}
, connectionResponseQueue(inNetwork.getDispatcher())
, spriteData{inSpriteData}
, chunkUpdateSystem(*this, world, network, inSpriteData)
, playerInputSystem(*this, world)
, serverUpdateSystem(*this, world, network)
, playerMovementSystem(*this, world, network)
, npcMovementSystem(*this, world, network, inSpriteData)
, cameraSystem(*this, world)
, currentTick(0)
{
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Simulation::connect()
{
    if (Config::RUN_OFFLINE) {
        // No need to connect if we're running offline. Just need mock player
        // data.
        fakeConnection();
        return;
    }

    while (!(network.connect())) {
        LOG_INFO("Network failed to connect. Retrying.");
    }

    // Wait for the data from the server.
    ConnectionResponse connectionResponse{};
    if (!(connectionResponseQueue.waitPop(connectionResponse,
                                          CONNECTION_RESPONSE_WAIT_US))) {
        LOG_ERROR("Server did not respond.");
    }

    // Save our player entity info.
    entt::entity playerEntity = connectionResponse.entity;
    LOG_INFO(
        "Received connection response. ID: %u, tick: %u, pos: (%.4f, %.4f)",
        playerEntity, connectionResponse.tickNum, connectionResponse.x,
        connectionResponse.y);

    // Resize the world's tile map.
    world.tileMap.setMapSize(connectionResponse.mapXLengthChunks
        , connectionResponse.mapYLengthChunks);
    LOG_INFO("Setting map size to: (%u, %u).", connectionResponse.mapXLengthChunks
        , connectionResponse.mapYLengthChunks);

    // Aim our tick for some reasonable point ahead of the server.
    // The server will adjust us after the first message anyway.
    currentTick = connectionResponse.tickNum + Config::INITIAL_TICK_OFFSET;

    // Create the player entity using the ID we received.
    entt::registry& registry = world.registry;
    entt::entity newEntity = registry.create(playerEntity);
    if (newEntity != playerEntity) {
        LOG_ERROR("Created entity doesn't match received entity. Created: %u, "
                  "received: %u",
                  newEntity, playerEntity);
    }

    // Save the player entity ID for convenience.
    world.playerEntity = newEntity;

    // Set up the player's sim components.
    registry.emplace<Name>(newEntity, std::to_string(static_cast<Uint32>(newEntity)));
    registry.emplace<Position>(newEntity, connectionResponse.x,
                               connectionResponse.y, 0.0f);
    registry.emplace<PreviousPosition>(newEntity, connectionResponse.x,
                                       connectionResponse.y, 0.0f);
    registry.emplace<Movement>(newEntity, 0.0f, 0.0f, 20.0f, 20.0f);
    registry.emplace<Input>(newEntity);

    // Set up the player's visual components.
    // TODO: Since sprite components hold static data, do we want them to
    //       just contain a ref to the actual data? Or is this copy better?
    registry.emplace<Sprite>(newEntity, spriteData.get("test_31"));
    registry.emplace<Camera>(newEntity, Camera::CenterOnEntity, Position{},
                             PreviousPosition{},
                             ScreenRect{0, 0, SharedConfig::SCREEN_WIDTH,
                                        SharedConfig::SCREEN_HEIGHT});

    // Set up the player's PlayerState component.
    registry.emplace<PlayerState>(newEntity);
}

void Simulation::fakeConnection()
{
    // Create the player entity.
    entt::registry& registry = world.registry;
    entt::entity newEntity = registry.create();

    // Save the player entity ID for convenience.
    world.playerEntity = newEntity;

    // Set up the player's sim components.
    registry.emplace<Name>(newEntity, std::to_string(static_cast<Uint32>(newEntity)));
    registry.emplace<Position>(newEntity, 0.0f, 0.0f, 0.0f);
    registry.emplace<PreviousPosition>(newEntity, 0.0f, 0.0f, 0.0f);
    registry.emplace<Movement>(newEntity, 0.0f, 0.0f, 20.0f, 20.0f);
    registry.emplace<Input>(newEntity);

    // Set up the player's visual components.
    registry.emplace<Sprite>(newEntity, spriteData.get("col_0"));
    registry.emplace<Camera>(newEntity, Camera::CenterOnEntity, Position{},
                             PreviousPosition{},
                             ScreenRect{0, 0, SharedConfig::SCREEN_WIDTH,
                                        SharedConfig::SCREEN_HEIGHT});

    // Set up the player's PlayerState component.
    registry.emplace<PlayerState>(newEntity);
}

void Simulation::tick()
{
    BEGIN_CPU_SAMPLE(SimTick);

    /* Calculate what tick we should be on. */
    // Increment the tick to the next.
    Uint32 targetTick = currentTick + 1;

    // If we're online, apply any adjustments that we receive from the
    // server.
    if (!Config::RUN_OFFLINE) {
        int adjustment = network.transferTickAdjustment();
        if (adjustment != 0) {
            targetTick += adjustment;

            // Make sure NPC replication takes the adjustment into account.
            npcMovementSystem.applyTickAdjustment(adjustment);
        }
    }

    /* Process ticks until we match what the server wants.
       This may cause us to not process any ticks, or to process multiple
       ticks. */
    while (currentTick < targetTick) {
        /* Run all systems. */
        // Process received chunk updates.
//        chunkUpdateSystem.updateChunks();

        // Process the held user input state.
        // Note: Mouse and momentary inputs are processed prior to this tick.
        playerInputSystem.processHeldInputs();

        // Send input change requests to the server.
        serverUpdateSystem.sendInputState();

        // Push the new input state into the player's history.
        playerInputSystem.addCurrentInputsToHistory();

        // Process player and NPC movements.
        playerMovementSystem.processMovements();

        // Process network movement after normal movement to sync with
        // server. (The server processes movement before sending updates.)
        npcMovementSystem.updateNpcs();

        // Move all cameras to their new positions.
        cameraSystem.moveCameras();

        currentTick++;
    }

    END_CPU_SAMPLE();
}

World& Simulation::getWorld()
{
    return world;
}

Uint32 Simulation::getCurrentTick()
{
    return currentTick;
}

bool Simulation::handleEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_MOUSEMOTION: {
            playerInputSystem.processMouseState(event.motion);
            return true;
        }
        default: {
            // Default to assuming it's a momentary input.
            playerInputSystem.processMomentaryInput(event);
            return true;
        }
    }

    return false;
}

} // namespace Client
} // namespace AM
