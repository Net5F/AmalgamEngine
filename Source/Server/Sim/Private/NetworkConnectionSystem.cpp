#include "NetworkConnectionSystem.h"
#include "Sim.h"
#include "World.h"
#include "Network.h"
#include "SimDefs.h"
#include "MessageTools.h"
#include "ConnectionResponse.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "ClientSimData.h"
#include "Name.h"
#include "Log.h"

namespace AM
{
namespace Server
{
NetworkConnectionSystem::NetworkConnectionSystem(Sim& inSim, World& inWorld,
                                                 Network& inNetwork)
: sim(inSim)
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
        entt::registry& registry = world.registry;
        const Position spawnPoint = world.getSpawnPoint();

        entt::entity newEntity = registry.create();
        registry.emplace<Name>(newEntity, std::to_string(static_cast<Uint32>(
                                              registry.version(newEntity))));
        registry.emplace<Position>(newEntity, spawnPoint.x, spawnPoint.y, 0.0f);
        registry.emplace<PreviousPosition>(newEntity, spawnPoint.x,
                                           spawnPoint.y, 0.0f);
        registry.emplace<Movement>(newEntity, 0.0f, 0.0f, 250.0f, 250.0f);
        registry.emplace<Input>(newEntity);
        registry.emplace<ClientSimData>(newEntity, clientNetworkID, false,
            AreaOfInterest{(SCREEN_WIDTH + AOI_BUFFER_DISTANCE), (SCREEN_HEIGHT
            + AOI_BUFFER_DISTANCE)});

        LOG_INFO("Constructed entity with netID: %u, entityID: %u",
                 clientNetworkID, newEntity);

        // Build and send the response.
        sendConnectionResponse(clientNetworkID, newEntity, spawnPoint.x,
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
        entt::entity clientEntity
            = world.findEntityWithNetID(disconnectedClientID);
        if (clientEntity != entt::null) {
            // Found the entity, remove it.
            world.registry.destroy(clientEntity);
            LOG_INFO("Removed entity with netID: %u", clientEntity);
        }
        else {
            LOG_ERROR("Failed to find entity with netID: %u while erasing.",
                      disconnectedClientID);
        }
    }
}

void NetworkConnectionSystem::sendConnectionResponse(NetworkID networkID,
                                                     entt::entity newEntity,
                                                     float spawnX, float spawnY)
{
    // Fill in their ID and spawn point.
    Uint32 currentTick = sim.getCurrentTick();
    ConnectionResponse connectionResponse{currentTick, newEntity, spawnX,
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
