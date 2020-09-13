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
, accumulatedTime(0.0)
, currentTick(0)
{
    world.setSpawnPoint({64, 64});
    Debug::registerCurrentTickPtr(&currentTick);
    network.registerCurrentTickPtr(&currentTick);
}

void Game::tick()
{
    accumulatedTime += iterationTimer.getDeltaSeconds(true);

    /* Process as many game ticks as have accumulated. */
    while (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
        // Add any newly connected clients to the sim.
        processConnectEvents();

        // Remove any newly disconnected clients from the sim.
        processDisconnectEvents();

        /* Run all systems. */
        networkInputSystem.processInputMessages();

        movementSystem.processMovements();

        networkOutputSystem.sendClientUpdates();

        /* Prepare for the next tick. */
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

        currentTick++;
    }
}

void Game::processConnectEvents()
{
    moodycamel::ReaderWriterQueue<NetworkID>& connectEventQueue =
        network.getConnectEventQueue();

    // Add all newly connected client's entities to the sim.
    for (unsigned int i = 0; i < connectEventQueue.size_approx(); ++i) {
        NetworkID clientNetworkID = 0;
        if (!(connectEventQueue.try_dequeue(clientNetworkID))) {
            DebugError("Expected element in connectEventQueue but dequeue failed.");
        }

        // Build their entity.
        EntityID newEntityID = world.addEntity("Player");
        const Position& spawnPoint = world.getSpawnPoint();
        world.positions[newEntityID].x = spawnPoint.x;
        world.positions[newEntityID].y = spawnPoint.y;
        world.movements[newEntityID].maxVelX = 250;
        world.movements[newEntityID].maxVelY = 250;
        world.clients.insert({newEntityID, {clientNetworkID}});
        world.attachComponent(newEntityID, ComponentFlag::Input);
        world.attachComponent(newEntityID, ComponentFlag::Movement);
        world.attachComponent(newEntityID, ComponentFlag::Position);
        world.attachComponent(newEntityID, ComponentFlag::Sprite);
        world.attachComponent(newEntityID, ComponentFlag::Client);

        DebugInfo("Constructed entity with netID: %u, entityID: %u", clientNetworkID,
            newEntityID);

        // Tell the network to send a connectionResponse on the next network tick.
        network.sendConnectionResponse(clientNetworkID, newEntityID, spawnPoint.x,
            spawnPoint.y);
    }
}

void Game::processDisconnectEvents()
{
    moodycamel::ReaderWriterQueue<NetworkID>& disconnectEventQueue =
        network.getDisconnectEventQueue();

    // Remove all newly disconnected client's entities from the sim.
    for (unsigned int i = 0; i < disconnectEventQueue.size_approx(); ++i) {
        NetworkID disconnectedClientID = 0;
        if (!(disconnectEventQueue.try_dequeue(disconnectedClientID))) {
            DebugError("Expected element in disconnectEventQueue but dequeue failed.");
        }

        // Iterate through the connected clients, bailing early if we find the one we want.
        bool entityFound = false;
        auto it = world.clients.begin();
        while ((it != world.clients.end()) && !entityFound) {
            if (it->second.networkID == disconnectedClientID) {
                // Found the ClientComponent we expected, remove the entity from everything.
                entityFound = true;

                world.removeEntity(it->first);
                DebugInfo("Erased entity with netID: %u", it->first);
                world.clients.erase(it);
            } else {
                ++it;
            }
        }

        if (!entityFound) {
            DebugError(
                "Failed to find entity with netID: %u while erasing.", disconnectedClientID);
        }
    }
}

void Game::initTimer()
{
    iterationTimer.updateSavedTime();
}

double Game::getAccumulatedTime()
{
    return accumulatedTime;
}

Uint32 Game::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
