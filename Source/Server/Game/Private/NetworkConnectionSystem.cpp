#include "NetworkConnectionSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "GameDefs.h"
#include "MessageTools.h"
#include "ConnectionResponse.h"
#include "Log.h"

namespace AM
{
namespace Server
{
NetworkConnectionSystem::NetworkConnectionSystem(Game& inGame, World& inWorld,
                                                 Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
{
}

void NetworkConnectionSystem::processConnectionEvents()
{
    processConnectEvents();

    processDisconnectEvents();
}

void NetworkConnectionSystem::processConnectEvents()
{
    moodycamel::ReaderWriterQueue<NetworkID>& connectEventQueue
        = network.getConnectEventQueue();

    // Add all newly connected client's entities to the sim.
    for (unsigned int i = 0; i < connectEventQueue.size_approx(); ++i) {
        NetworkID clientNetworkID = 0;
        if (!(connectEventQueue.try_dequeue(clientNetworkID))) {
            LOG_ERROR(
                "Expected element in connectEventQueue but dequeue failed.");
        }

        // Build their entity.
        EntityID newEntityID = world.addEntity("Player");
        const Position& spawnPoint = world.getSpawnPoint();
        world.positions[newEntityID].x = spawnPoint.x;
        world.positions[newEntityID].y = spawnPoint.y;
        world.movements[newEntityID].maxVelX = 250;
        world.movements[newEntityID].maxVelY = 250;
        world.clients[newEntityID].netID = clientNetworkID;
        world.clients[newEntityID].isInitialized = false;
        world.attachComponent(newEntityID, ComponentFlag::Input);
        world.attachComponent(newEntityID, ComponentFlag::Movement);
        world.attachComponent(newEntityID, ComponentFlag::Position);
        world.attachComponent(newEntityID, ComponentFlag::Sprite);
        world.attachComponent(newEntityID, ComponentFlag::Client);

        LOG_INFO("Constructed entity with netID: %u, entityID: %u",
                 clientNetworkID, newEntityID);

        // Build and send the response.
        sendConnectionResponse(clientNetworkID, newEntityID, spawnPoint.x,
                               spawnPoint.y);
    }
}

void NetworkConnectionSystem::processDisconnectEvents()
{
    moodycamel::ReaderWriterQueue<NetworkID>& disconnectEventQueue
        = network.getDisconnectEventQueue();

    // Remove all newly disconnected client's entities from the sim.
    for (unsigned int i = 0; i < disconnectEventQueue.size_approx(); ++i) {
        NetworkID disconnectedClientID = 0;
        if (!(disconnectEventQueue.try_dequeue(disconnectedClientID))) {
            LOG_ERROR(
                "Expected element in disconnectEventQueue but dequeue failed.");
        }

        // Find the client's associated entity.
        bool entityFound = false;
        for (std::size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
            if ((world.componentFlags[entityID] & ComponentFlag::Client)
            && (world.clients[entityID].netID == disconnectedClientID)) {
                // Found the entity, remove it.
                world.removeEntity(entityID);
                entityFound = true;
                LOG_INFO("Removed entity with netID: %u", entityID);
            }
        }

        if (!entityFound) {
            LOG_ERROR("Failed to find entity with netID: %u while erasing.",
                      disconnectedClientID);
        }
    }
}

void NetworkConnectionSystem::sendConnectionResponse(NetworkID networkID,
                                                     EntityID newEntityID,
                                                     float spawnX, float spawnY)
{
    // Fill in their ID and spawn point.
    Uint32 currentTick = game.getCurrentTick();
    ConnectionResponse connectionResponse{currentTick, newEntityID, spawnX,
                                          spawnY};

    // Serialize the connection response message.
    BinaryBufferSharedPtr messageBuffer
        = std::make_shared<BinaryBuffer>(Peer::MAX_MESSAGE_SIZE);
    std::size_t messageSize = MessageTools::serialize(
        *messageBuffer, connectionResponse, MESSAGE_HEADER_SIZE);

    // Fill the buffer with the appropriate message header.
    MessageTools::fillMessageHeader(MessageType::ConnectionResponse,
                                    messageSize, messageBuffer, 0);

    // Send the message.
    network.send(networkID, messageBuffer, currentTick);
}

} // namespace Server
} // namespace AM
