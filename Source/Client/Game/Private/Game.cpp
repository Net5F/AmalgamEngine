#include "Game.h"
#include "Network.h"
#include "ClientNetworkDefs.h"
#include "ConnectionResponse.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Client
{
Game::Game(Network& inNetwork,
           const std::shared_ptr<SDL2pp::Texture>& inSprites)
: world()
, network(inNetwork)
, playerInputSystem(*this, world)
, networkUpdateSystem(*this, world, network)
, playerMovementSystem(*this, world, network)
, npcMovementSystem(*this, world, network)
, accumulatedTime(0.0)
, currentTick(0)
, sprites(inSprites)
, exitRequested(false)
{
    Log::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Game::connect()
{
    if (RUN_OFFLINE) {
        // No need to connect if we're running offline. Just need mock player
        // data.
        fakeConnection();
        return;
    }

    while (!(network.connect())) {
        LOG_INFO("Network failed to connect. Retrying.");
    }

    // Wait for the player's ID from the server.
    std::unique_ptr<ConnectionResponse> connectionResponse
        = network.receiveConnectionResponse(CONNECTION_RESPONSE_WAIT_MS);
    if (connectionResponse == nullptr) {
        LOG_ERROR("Server did not respond.");
    }

    // Get our info from the connection response.
    EntityID playerID = connectionResponse->entityID;
    LOG_INFO("Received connection response. ID: %u, tick: %u", playerID,
             connectionResponse->tickNum);

    // Aim our tick for some reasonable point ahead of the server.
    // The server will adjust us after the first message anyway.
    currentTick = connectionResponse->tickNum + INITIAL_TICK_OFFSET;

    // Set up our player.
    SDL2pp::Rect textureRect(0, 32, 16, 16);

    // Register the player ID with the world and the network.
    world.registerPlayerID(playerID);

    // Initialize the player state.
    world.addEntity("Player", playerID);
    world.positions[playerID].x = connectionResponse->x;
    world.positions[playerID].y = connectionResponse->y;
    world.oldPositions[playerID].x = world.positions[playerID].x;
    world.oldPositions[playerID].y = world.positions[playerID].y;
    world.movements[playerID].maxVelX = 250;
    world.movements[playerID].maxVelY = 250;
    world.inputs[playerID].inputStates = {};
    world.sprites[playerID].texturePtr = sprites;
    world.sprites[playerID].posInTexture = textureRect;
    world.sprites[playerID].width = 64;
    world.sprites[playerID].height = 64;
    world.attachComponent(playerID, ComponentFlag::Input);
    world.attachComponent(playerID, ComponentFlag::Movement);
    world.attachComponent(playerID, ComponentFlag::Position);
    world.attachComponent(playerID, ComponentFlag::Sprite);
}

void Game::fakeConnection()
{
    // Set up our player.
    SDL2pp::Rect textureRect(0, 32, 16, 16);

    // Register the player ID with the world and the network.
    EntityID player = 0;
    world.registerPlayerID(player);

    // Initialize the player state.
    world.addEntity("Player", player);
    world.positions[player].x = 64;
    world.positions[player].y = 64;
    world.oldPositions[player].x = world.positions[player].x;
    world.oldPositions[player].y = world.positions[player].y;
    world.movements[player].maxVelX = 250;
    world.movements[player].maxVelY = 250;
    world.sprites[player].texturePtr = sprites;
    world.sprites[player].posInTexture = textureRect;
    world.sprites[player].width = 64;
    world.sprites[player].height = 64;
    world.attachComponent(player, ComponentFlag::Input);
    world.attachComponent(player, ComponentFlag::Movement);
    world.attachComponent(player, ComponentFlag::Position);
    world.attachComponent(player, ComponentFlag::Sprite);
}

void Game::tick()
{
    accumulatedTime += iterationTimer.getDeltaSeconds(true);

    // Process as many game ticks as have accumulated.
    while (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
        /* Calculate what tick we should be on. */
        // Increment the tick to the next.
        Uint32 targetTick = currentTick + 1;

        // If we're online, apply any adjustments that we receive from the
        // server.
        if (!RUN_OFFLINE) {
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
            // Process all waiting user input events.
            processUserInputEvents();

            // Send input updates to the server.
            networkUpdateSystem.sendInputState();

            // Push the new input state into the player's history.
            playerInputSystem.addCurrentInputsToHistory();

            // Process player and NPC movements.
            playerMovementSystem.processMovements();

            // Process network movement after normal movement to sync with
            // server. (The server processes movement before sending updates.)
            npcMovementSystem.updateNpcs();

            currentTick++;
        }

        accumulatedTime -= GAME_TICK_TIMESTEP_S;
        if (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
            LOG_INFO("Detected a request for multiple game ticks in the same "
                     "frame. Game tick "
                     "must have been massively delayed. Game tick was delayed "
                     "by: %.8fs.",
                     accumulatedTime);
        }
        else if (accumulatedTime >= GAME_DELAYED_TIME_S) {
            // Game missed its ideal call time, could be our issue or general
            // system slowness.
            LOG_INFO("Detected a delayed game tick. Game tick was delayed by: "
                     "%.8fs.",
                     accumulatedTime);
        }

        // Check our execution time.
        double executionTime = iterationTimer.getDeltaSeconds(false);
        if (executionTime > GAME_TICK_TIMESTEP_S) {
            LOG_INFO("Overran our sim iteration time. executionTime: %.8f",
                     executionTime);
        }
    }
}

void Game::processUserInputEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            exitRequested = true;
        }
        else if (event.type == SDL_WINDOWEVENT) {
            //            switch(event.type) {
            //                case SDL_WINDOWEVENT_SHOWN:
            //                case SDL_WINDOWEVENT_EXPOSED:
            //                case SDL_WINDOWEVENT_MOVED:
            //                case SDL_WINDOWEVENT_MAXIMIZED:
            //                case SDL_WINDOWEVENT_RESTORED:
            //                case SDL_WINDOWEVENT_FOCUS_GAINED:
            //                // Window was messed with, we've probably lost
            //                sync with the server.
            //                // TODO: Handle the far-out-of-sync client.
            //            }
        }
        else {
            // Assume it's a key or mouse event.
            playerInputSystem.processMomentaryInput(event);
        }
    }

    // Process held inputs (movement, etc).
    playerInputSystem.processHeldInputs();
}

void Game::initTimer()
{
    iterationTimer.updateSavedTime();
}

World& Game::getWorld()
{
    return world;
}

double Game::getTimeTillNextIteration()
{
    // The time since accumulatedTime was last updated.
    double timeSinceIteration = iterationTimer.getDeltaSeconds(false);
    return (GAME_TICK_TIMESTEP_S - (accumulatedTime + timeSinceIteration));
}

double Game::getIterationProgress()
{
    // The time since accumulatedTime was last updated.
    double timeSinceIteration = iterationTimer.getDeltaSeconds(false);
    return ((accumulatedTime + timeSinceIteration) / GAME_TICK_TIMESTEP_S);
}

Uint32 Game::getCurrentTick()
{
    return currentTick;
}

std::atomic<bool> const* Game::getExitRequestedPtr()
{
    return &exitRequested;
}

} // namespace Client
} // namespace AM
