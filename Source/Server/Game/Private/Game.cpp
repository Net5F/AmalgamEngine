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

    /* Process as many game ticks as have accumulated. */
    while (accumulatedTime >= GAME_TICK_INTERVAL_S) {
        /* Add any newly connected clients to the game sim. */
        moodycamel::ReaderWriterQueue<NetworkID>& connectEventQueue =
            network.getConnectEventQueue();

        // TODO: Move to a function to make this loop easier to read.
        for (unsigned int i = 0; i < connectEventQueue.size_approx(); ++i) {
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
            // TODO: Add NetworkComponent that tracks NetworkID.

            // Tell the network to send a connectionResponse on the next network tick.
            // TODO: Refactor this to account for NetworkID and EntityID.
            network.sendConnectionResponse(newID, spawnPoint.x, spawnPoint.y);
        }

        // TODO: Process disconnects.

        /* Run all systems. */
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
