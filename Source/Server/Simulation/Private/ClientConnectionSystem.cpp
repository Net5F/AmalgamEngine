#include "ClientConnectionSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SharedConfig.h"
#include "Serialize.h"
#include "ConnectionResponse.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "ClientSimData.h"
#include "Name.h"
#include "Log.h"
#include "Profiler.h"

namespace AM
{
namespace Server
{
ClientConnectionSystem::ClientConnectionSystem(Simulation& inSim,
                                                 World& inWorld,
                                                 Network& inNetwork)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
, clientConnectedQueue(inNetwork.getDispatcher())
, clientDisconnectedQueue(inNetwork.getDispatcher())
{
}

void ClientConnectionSystem::processConnectionEvents()
{
    SCOPED_CPU_SAMPLE(processConnectionEvents);

    processConnectEvents();

    processDisconnectEvents();
}

void ClientConnectionSystem::processConnectEvents()
{
    // Add all newly connected client's entities to the sim.
    for (unsigned int i = 0; i < clientConnectedQueue.size(); ++i) {
        ClientConnected clientConnected{};
        if (!(clientConnectedQueue.pop(clientConnected))) {
            LOG_ERROR("Expected element but pop failed.");
        }

        // Build their entity.
        entt::registry& registry = world.registry;
        const Position spawnPoint = world.getGroupedSpawnPoint();

        entt::entity newEntity = registry.create();
        registry.emplace<Name>(newEntity, std::to_string(static_cast<Uint32>(newEntity)));
        registry.emplace<Position>(newEntity, spawnPoint.x, spawnPoint.y, 0.0f);
        registry.emplace<PreviousPosition>(newEntity, spawnPoint.x,
                                           spawnPoint.y, 0.0f);
        registry.emplace<Movement>(newEntity, 0.0f, 0.0f, 250.0f, 250.0f);
        registry.emplace<Input>(newEntity);
        registry.emplace<ClientSimData>(
            newEntity, clientConnected.clientID, false, true,
            AreaOfInterest{(SharedConfig::SCREEN_WIDTH
                            + SharedConfig::AOI_BUFFER_DISTANCE),
                           (SharedConfig::SCREEN_HEIGHT
                            + SharedConfig::AOI_BUFFER_DISTANCE)});
        world.netIdMap[clientConnected.clientID] = newEntity;

        LOG_INFO("Constructed entity with netID: %u, entityID: %u",
                 clientConnected.clientID, newEntity);

        // Build and send the response.
        sendConnectionResponse(clientConnected.clientID, newEntity, spawnPoint.x,
                               spawnPoint.y);
    }
}

void ClientConnectionSystem::processDisconnectEvents()
{
    // Remove all newly disconnected client's entities from the sim.
    for (unsigned int i = 0; i < clientDisconnectedQueue.size(); ++i) {
        ClientDisconnected clientDisconnected{};
        if (!(clientDisconnectedQueue.pop(clientDisconnected))) {
            LOG_ERROR("Expected element but pop failed.");
        }

        // Find the client's associated entity.
        auto clientEntityIt = world.netIdMap.find(clientDisconnected.clientID);
        if (clientEntityIt != world.netIdMap.end()) {
            // Found the entity, remove it.
            entt::entity clientEntity = clientEntityIt->second;
            world.registry.destroy(clientEntity);
            world.netIdMap.erase(clientEntityIt);
            LOG_INFO("Removed entity with entityID: %u", clientEntity);
        }
        else {
            LOG_ERROR("Failed to find entity with netID: %u while erasing.",
                      clientDisconnected.clientID);
        }
    }
}

void ClientConnectionSystem::sendConnectionResponse(NetworkID networkID,
                                                     entt::entity newEntity,
                                                     float spawnX, float spawnY)
{
    // Fill in their ID and spawn point.
    Uint32 currentTick = sim.getCurrentTick();
    ConnectionResponse connectionResponse{currentTick, newEntity, spawnX,
                                          spawnY};

    // Send the connection response message.
    network.serializeAndSend(networkID, connectionResponse, currentTick);
}

} // namespace Server
} // namespace AM
