#include "NetworkConnectionSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "GameDefs.h"
#include "MessageUtil.h"
#include "Debug.h"

namespace AM
{
namespace Server
{

NetworkConnectionSystem::NetworkConnectionSystem(Game& inGame, World& inWorld,
                                                 Network& inNetwork)
: game(inGame), world(inWorld), network(inNetwork), builder(BUILDER_BUFFER_SIZE)
{
}

void NetworkConnectionSystem::processConnectionEvents() {
    processConnectEvents();

    processDisconnectEvents();
}

void NetworkConnectionSystem::processConnectEvents()
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

        // Build and send the response.
        sendConnectionResponse(clientNetworkID, newEntityID, spawnPoint.x,
                               spawnPoint.y);
    }
}

void NetworkConnectionSystem::processDisconnectEvents()
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

void NetworkConnectionSystem::sendConnectionResponse(NetworkID networkID,
                                                     EntityID newEntityID, float spawnX,
                                                     float spawnY)
{
    // Prep the builder for a new message.
    builder.Clear();

    // Fill in their ID and spawn point.
    Uint32 latestTickTimestamp = game.getCurrentTick();
    auto response = fb::CreateConnectionResponse(builder, newEntityID,
        spawnX, spawnY);
    auto encodedMessage = fb::CreateMessage(builder, latestTickTimestamp,
        fb::MessageContent::ConnectionResponse, response.Union());
    builder.Finish(encodedMessage);

    // Send the message.
    network.send(networkID,
        Network::constructMessage(builder.GetBufferPointer(), builder.GetSize()));
}

} // namespace Server
} // namespace AM
