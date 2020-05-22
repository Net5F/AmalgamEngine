#include "Game.h"
#include "Network.h"
#include "Debug.h"

namespace AM
{
namespace Server
{

Game::Game(Network& inNetwork)
: world()
, network(inNetwork)
, networkInputSystem(*this, world, network)
, movementSystem(world)
, networkOutputSystem(*this, world, network)
, accumulatedTime(0.0f)
, currentTick(0)
{
    world.setSpawnPoint({64, 64});
    Debug::registerCurrentTickPtr(&currentTick);
    Network::registerCurrentTickPtr(&currentTick);
}

void Game::tick(float deltaSeconds)
{
    accumulatedTime += deltaSeconds;

    // Process as many game ticks as have accumulated.
    while (accumulatedTime >= GAME_TICK_INTERVAL_S) {
        // Add any new connections.
        std::shared_ptr<Peer> newClient = network.getNewClient();
        while (newClient != nullptr) {
            // Build their entity.
            EntityID newID = world.addEntity("Player");
            const Position& spawnPoint = world.getSpawnPoint();
            world.positions[newID].x = spawnPoint.x;
            world.positions[newID].y = spawnPoint.y;
            world.movements[newID].maxVelX = 250;
            world.movements[newID].maxVelY = 250;
            world.attachComponent(newID, ComponentFlag::Input);
            world.attachComponent(newID, ComponentFlag::Movement);
            world.attachComponent(newID, ComponentFlag::Position);
            world.attachComponent(newID, ComponentFlag::Sprite);

            // Add them to the network's map.
            network.addClient(newID, newClient);

            // Tell the network to send a connectionResponse on the next network tick.
            network.sendConnectionResponse(newID, spawnPoint.x, spawnPoint.y);

            newClient = network.getNewClient();
        }

        // Run all systems.
        networkInputSystem.processInputMessages();

        movementSystem.processMovements(GAME_TICK_INTERVAL_S);

        networkOutputSystem.broadcastDirtyEntities();

        accumulatedTime -= GAME_TICK_INTERVAL_S;

        currentTick++;
    }
}

Uint32 Game::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
