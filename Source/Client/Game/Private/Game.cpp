#include "Game.h"
#include "Network.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

Game::Game(Network& inNetwork, const std::shared_ptr<SDL2pp::Texture>& inSprites)
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
    Debug::registerCurrentTickPtr(&currentTick);
}

void Game::connect()
{
    if (Network::RUN_OFFLINE) {
        // No need to connect if we're running offline. Just need mock player data.
        fakeConnection();
        return;
    }

    while (!(network.connect())) {
        DebugInfo("Network failed to connect. Retrying.");
    }

    // Wait for the player's ID from the server.
    BinaryBufferSharedPtr responseBuffer = network.receiveConnectionResponse(
                                                   CONNECTION_RESPONSE_WAIT_MS);
    if (responseBuffer == nullptr) {
        DebugError("Server did not respond.");
    }

    // Get our info from the connection response.
    const fb::Message* message = fb::GetMessage(responseBuffer->data());
    auto connectionResponse = static_cast<const fb::ConnectionResponse*>(message->content());
    EntityID player = connectionResponse->entityID();
    DebugInfo("Received connection response. ID: %u, tick: %u"
              , player, message->tickTimestamp());

    // Aim our tick for some reasonable point ahead of the server.
    // The server will adjust us after the first message anyway.
    currentTick = message->tickTimestamp() + Network::INITIAL_TICK_OFFSET;

    // Set up our player.
    SDL2pp::Rect textureRect(0, 32, 16, 16);

    // Register the player ID with the world and the network.
    world.registerPlayerID(player);

    // Initialize the player state.
    world.addEntity("Player", player);
    world.positions[player].x = connectionResponse->x();
    world.positions[player].y = connectionResponse->y();
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
        // Calculate what tick we should be on.
        Uint32 targetTick = currentTick + 1;
        if (!Network::RUN_OFFLINE) {
            // If we're online, apply any adjustments that we receive from the server.
            targetTick += network.transferTickAdjustment();
        }

        /* Process ticks until we match what the server wants.
           This may cause us to not process any ticks, or to process multiple ticks. */
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

            // Process network movement after normal movement to sync with server.
            // (The server processes movement before sending updates.)
            npcMovementSystem.updateNpcs();

            currentTick++;
        }

        accumulatedTime -= GAME_TICK_TIMESTEP_S;
        if (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
            DebugInfo(
                "Detected a request for multiple game ticks in the same frame. Game tick "
                "must have been massively delayed. Game tick was delayed by: %.8fs.",
                accumulatedTime);
        }
        else if (accumulatedTime >= GAME_DELAYED_TIME_S) {
            // Game missed its ideal call time, could be our issue or general system slowness.
            DebugInfo("Detected a delayed game tick. Game tick was delayed by: %.8fs.",
                accumulatedTime);
        }

        // Check our execution time.
        double executionTime = iterationTimer.getDeltaSeconds(false);
        if (executionTime > GAME_TICK_TIMESTEP_S) {
            DebugInfo("Overran our sim iteration time. executionTime: %.8f",
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
//                // Window was messed with, we've probably lost sync with the server.
//                // TODO: Handle the far-out-of-sync client.
//            }
        }
        else {
            // Assume it's a key or mouse event.
            playerInputSystem.processInputEvent(event);
        }
    }
}

void Game::initTimer()
{
    iterationTimer.updateSavedTime();
}

World& Game::getWorld()
{
    return world;
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

std::atomic<bool> const* Game::getExitRequestedPtr() {
    return &exitRequested;
}

} // namespace Client
} // namespace AM
