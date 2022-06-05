#include "ClientConnectionSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "SharedConfig.h"
#include "Serialize.h"
#include "ConnectionResponse.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "ClientSimData.h"
#include "BoundingBox.h"
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
    Simulation& inSim, World& inWorld,
    EventDispatcher& inNetworkEventDispatcher, Network& inNetwork,
    SpriteData& inSpriteData)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
, spriteData{inSpriteData}
, clientConnectedQueue(inNetworkEventDispatcher)
, clientDisconnectedQueue(inNetworkEventDispatcher)
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
    for (unsigned int i = 0; i < clientConnectedQueue.size(); ++i) {
        ClientConnected clientConnected{};
        if (!(clientConnectedQueue.pop(clientConnected))) {
            LOG_FATAL("Expected element but pop failed.");
        }

        /* Build their entity. */
        // Find their spawn point.
        entt::registry& registry{world.registry};
        const Position spawnPoint{world.getRandomSpawnPoint()};

        // Create the entity and construct its standard components.
        entt::entity newEntity{registry.create()};
        registry.emplace<Name>(newEntity,
                               std::to_string(static_cast<Uint32>(newEntity)));
        Position& newPosition{registry.emplace<Position>(
            newEntity, spawnPoint.x, spawnPoint.y, 0.0f)};
        registry.emplace<PreviousPosition>(newEntity, spawnPoint.x,
                                           spawnPoint.y, 0.0f);
        registry.emplace<Velocity>(newEntity, 0.0f, 0.0f, 250.0f, 250.0f);
        registry.emplace<Input>(newEntity);
        registry.emplace<ClientSimData>(newEntity, clientConnected.clientID,
                                        false, std::vector<entt::entity>());
        Sprite& newSprite{registry.emplace<Sprite>(
            newEntity, spriteData.get(SharedConfig::DEFAULT_CHARACTER_SPRITE))};
        BoundingBox& boundingBox{registry.emplace<BoundingBox>(
            newEntity, Transforms::modelToWorldCentered(newSprite.modelBounds,
                                                        newPosition))};

        // Start tracking the entity in the locator.
        world.entityLocator.setEntityLocation(newEntity, boundingBox);

        // Register the entity with the network ID map.
        world.netIdMap[clientConnected.clientID] = newEntity;

        LOG_INFO("Constructed entity with netID: %u, entityID: %u",
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
    for (unsigned int i = 0; i < clientDisconnectedQueue.size(); ++i) {
        ClientDisconnected clientDisconnected{};
        if (!(clientDisconnectedQueue.pop(clientDisconnected))) {
            LOG_FATAL("Expected element but pop failed.");
        }

        // Find the disconnected client's associated entity.
        auto disconnectedEntityIt{
            world.netIdMap.find(clientDisconnected.clientID)};
        if (disconnectedEntityIt != world.netIdMap.end()) {
            // Found the entity. Remove it from the entity locator.
            entt::entity disconnectedEntity{disconnectedEntityIt->second};
            world.entityLocator.removeEntity(disconnectedEntity);

            // Remove it from the AOI lists of all client entities in its
            // range and send them EntityDeletes.
            ClientSimData& disconnectedClient{
                view.get<ClientSimData>(disconnectedEntity)};
            for (entt::entity entityInRange :
                 disconnectedClient.entitiesInAOI) {
                // If entityInRange isn't a client entity, skip it.
                if (!(world.registry.all_of<ClientSimData>(entityInRange))) {
                    continue;
                }

                // Find disconnectedEntity in entityInRange's list.
                ClientSimData& clientInRange{
                    view.get<ClientSimData>(entityInRange)};
                auto eraseIt{std::find(clientInRange.entitiesInAOI.begin(),
                                       clientInRange.entitiesInAOI.end(),
                                       disconnectedEntity)};

                // Remove disconnectedEntity from entityInRange's list.
                if (eraseIt != clientInRange.entitiesInAOI.end()) {
                    clientInRange.entitiesInAOI.erase(eraseIt);
                }
                else {
                    LOG_FATAL("Failed to find expected entity when erasing.");
                }

                // Tell clientInRange that disconnectedEntity has been deleted.
                network.serializeAndSend(
                    clientInRange.netID,
                    EntityDelete{sim.getCurrentTick(), disconnectedEntity});
            }

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
    Uint32 currentTick{sim.getCurrentTick()};
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
