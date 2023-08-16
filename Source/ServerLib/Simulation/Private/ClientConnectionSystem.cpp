#include "ClientConnectionSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "SharedConfig.h"
#include "Serialize.h"
#include "ConnectionResponse.h"
#include "Input.h"
#include "Rotation.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "ClientSimData.h"
#include "EntityType.h"
#include "Collision.h"
#include "Name.h"
#include "EntityDelete.h"
#include "Transforms.h"
#include "Log.h"
#include "Tracy.hpp"

namespace AM
{
namespace Server
{
ClientConnectionSystem::ClientConnectionSystem(
    Simulation& inSimulation, World& inWorld,
    EventDispatcher& inNetworkEventDispatcher, Network& inNetwork,
    SpriteData& inSpriteData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, clientConnectedQueue{inNetworkEventDispatcher}
, clientDisconnectedQueue{inNetworkEventDispatcher}
{
}

void ClientConnectionSystem::processConnectionEvents()
{
    ZoneScoped;

    processConnectEvents();

    processDisconnectEvents();
}

void ClientConnectionSystem::processConnectEvents()
{
    // Add all newly connected client's entities to the sim.
    for (std::size_t i = 0; i < clientConnectedQueue.size(); ++i) {
        ClientConnected clientConnected{};
        if (!(clientConnectedQueue.pop(clientConnected))) {
            LOG_FATAL("Expected element but pop failed.");
        }

        /* Build their entity. */
        // Find their spawn point.
        entt::registry& registry{world.registry};
        const Position spawnPoint{world.getSpawnPoint()};

        // Create the entity and construct its standard components.
        // Note: Be careful with holding onto references here. If components 
        //       are added to the same group, the ref will be invalidated.
        entt::entity newEntity{registry.create()};
        registry.emplace<EntityType>(newEntity, EntityType::ClientEntity);
        registry.emplace<Name>(newEntity,
                               std::to_string(static_cast<Uint32>(newEntity)));

        registry.emplace<Input>(newEntity);
        registry.emplace<Position>(newEntity, spawnPoint.x, spawnPoint.y, 0.0f);
        registry.emplace<PreviousPosition>(newEntity, spawnPoint.x,
                                           spawnPoint.y, 0.0f);
        registry.emplace<Velocity>(newEntity, 0.0f, 0.0f, 250.0f, 250.0f);
        registry.emplace<Rotation>(newEntity);

        registry.emplace<ClientSimData>(newEntity, clientConnected.clientID,
                                        std::vector<entt::entity>());
        const Sprite& sprite{registry.emplace<Sprite>(
            newEntity,
            spriteData.getSprite(SharedConfig::DEFAULT_CHARACTER_SPRITE))};
        const Collision& collision{registry.emplace<Collision>(
            newEntity, sprite.modelBounds,
            Transforms::modelToWorldCentered(
                sprite.modelBounds, registry.get<Position>(newEntity)))};

        // Start tracking the entity in the locator.
        // Note: Since the entity was added to the locator, clients 
        //       will be told by ClientAOISystem to construct it.
        world.entityLocator.setEntityLocation(newEntity,
                                              collision.worldBounds);

        // Add the new client entity to the network ID map.
        world.netIdMap[clientConnected.clientID] = newEntity;

        LOG_INFO("Constructed client entity with netID: %u, entityID: %u",
                 clientConnected.clientID, newEntity);

        // Build and send the response.
        sendConnectionResponse(clientConnected.clientID, newEntity,
                               spawnPoint.x, spawnPoint.y);
    }
}

void ClientConnectionSystem::processDisconnectEvents()
{
    auto view{world.registry.view<ClientSimData>()};

    // Remove all newly disconnected client's entities from the sim.
    for (std::size_t i = 0; i < clientDisconnectedQueue.size(); ++i) {
        ClientDisconnected clientDisconnected{};
        if (!(clientDisconnectedQueue.pop(clientDisconnected))) {
            LOG_FATAL("Expected element but pop failed.");
        }

        // Find the disconnected client's associated entity.
        auto disconnectedEntityIt{
            world.netIdMap.find(clientDisconnected.clientID)};
        if (disconnectedEntityIt != world.netIdMap.end()) {
            // Found the entity. Remove it from the entity locator.
            // Note: Since the entity was removed from the locator, its peers
            //       will be told by ClientAOISystem to delete it.
            entt::entity disconnectedEntity{disconnectedEntityIt->second};
            world.entityLocator.removeEntity(disconnectedEntity);

            // Remove it from the registry and network ID map.
            world.registry.destroy(disconnectedEntity);
            world.netIdMap.erase(disconnectedEntityIt);

            LOG_INFO("Removed entity with entityID: %u", disconnectedEntity);
        }
        else {
            LOG_FATAL("Failed to find entity with netID: %u while erasing.",
                      clientDisconnected.clientID);
        }
    }
}

void ClientConnectionSystem::sendConnectionResponse(NetworkID networkID,
                                                    entt::entity newEntity,
                                                    float spawnX, float spawnY)
{
    // Fill in the current tick and their entity's ID.
    ConnectionResponse connectionResponse{};
    Uint32 currentTick{simulation.getCurrentTick()};
    connectionResponse.entity = newEntity;
    connectionResponse.tickNum = currentTick;

    // Fill in their spawn point.
    connectionResponse.x = spawnX;
    connectionResponse.y = spawnY;

    // Fill in the map's size.
    const ChunkExtent& mapChunkExtent{world.tileMap.getChunkExtent()};
    connectionResponse.mapXLengthChunks = mapChunkExtent.xLength;
    connectionResponse.mapYLengthChunks = mapChunkExtent.yLength;

    // Send the connection response message.
    network.serializeAndSend(networkID, connectionResponse, currentTick);
}

} // namespace Server
} // namespace AM
