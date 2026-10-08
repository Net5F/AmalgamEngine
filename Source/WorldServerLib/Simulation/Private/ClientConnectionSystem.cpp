#include "ClientConnectionSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "Network.h"
#include "GraphicData.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Serialize.h"
#include "ConnectionResponse.h"
#include "Input.h"
#include "Rotation.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Name.h"
#include "Inventory.h"
#include "ClientSimData.h"
#include "IsClientEntity.h"
#include "GraphicState.h"
#include "VariantTools.h"
#include "Log.h"
#include "tracy/Tracy.hpp"

namespace AM
{
namespace WorldServer
{
ClientConnectionSystem::ClientConnectionSystem(
    const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, graphicData{inSimContext.graphicData}
, clientConnectionEventQueue{network.clientEndpoint.getEventDispatcher()}
, accountEntityMap{}
, removalTickMap{}
{
}

void ClientConnectionSystem::processConnectionEvents()
{
    ZoneScoped;

    // Process all newly connected or disconnected entities.
    for (std::size_t i{0}; i < clientConnectionEventQueue.size(); ++i) {
        ClientConnectionEvent clientConnectionEvent{};
        if (!(clientConnectionEventQueue.pop(clientConnectionEvent))) {
            LOG_ERROR("Expected element but pop failed.");
        }

        std::visit(VariantTools::Overload{
                       [&](const ClientConnected& clientConnected) {
                           processConnectEvent(clientConnected);
                       },
                       [&](const ClientDisconnected& clientDisconnected) {
                           processDisconnectEvent(clientDisconnected);
                       }},
                   clientConnectionEvent);
    }

    // Remove any entities that finished lingering.
    removeExpiredEntities();
}

void ClientConnectionSystem::processConnectEvent(
    const ClientConnected& clientConnected)
{
    // If this account's entity is still in the world, take it over.
    auto entityIt{accountEntityMap.find(clientConnected.accountID)};
    if ((entityIt != accountEntityMap.end())
        && world.registry.valid(entityIt->second)) {
        attachClientToEntity(clientConnected, entityIt->second);
    }
    else {
        accountEntityMap[clientConnected.accountID]
            = createClientEntity(clientConnected);
    }
}

void ClientConnectionSystem::processDisconnectEvent(
    const ClientDisconnected& clientDisconnected)
{
    // Find the disconnected client's associated entity.
    auto disconnectedEntityIt{world.netIDMap.find(clientDisconnected.clientID)};
    if (disconnectedEntityIt == world.netIDMap.end()) {
        LOG_ERROR("Failed to find entity with netID: %u while disconnecting.",
                  clientDisconnected.clientID);
        return;
    }

    // Detach the client from its entity.
    entt::entity entity{disconnectedEntityIt->second};
    world.netIDMap.erase(disconnectedEntityIt);
    ClientSimData& client{world.registry.get<ClientSimData>(entity)};
    client.netID = NULL_NETWORK_ID;
    Sint64 accountID{client.accountID};

    // If entities don't linger, remove the entity immediately.
    if (Config::DISCONNECT_LINGER_TICKS == 0) {
        removeClientEntity(accountID);
        return;
    }

    // Release any inputs the client was holding, so the entity doesn't keep
    // moving while it waits to be removed.
    world.registry.replace<Input>(entity);

    // Start lingering.
    removalTickMap[accountID]
        = simulation.getCurrentTick() + Config::DISCONNECT_LINGER_TICKS;
    LOG_INFO("Client disconnected, entity is lingering. netID: %u, "
             "entityID: %u",
             clientDisconnected.clientID, entity);
}

entt::entity ClientConnectionSystem::createClientEntity(
    const ClientConnected& clientConnected)
{
    entt::registry& registry{world.registry};

    // Create the entity and construct its standard components.
    // Note: Be careful with holding onto references here. If components
    //       are added to the same group, the ref will be invalidated.
    entt::entity newEntity{world.createEntity(world.getSpawnPoint())};

    // TODO: When player accounts are added, a lot of this should be changed to
    //       load the persisted data from the account.
    registry.emplace<IsClientEntity>(newEntity);
    registry.emplace<Name>(
        newEntity, "Player " + std::to_string(static_cast<Uint32>(newEntity)));
    registry.emplace<Inventory>(newEntity);

    registry.emplace<ClientSimData>(newEntity, clientConnected.clientID,
                                    clientConnected.accountID);

    world.addMovementComponents(newEntity);

    const EntityGraphicSet& graphicSet{graphicData.getEntityGraphicSet(
        SharedConfig::DEFAULT_ENTITY_GRAPHIC_SET)};
    GraphicState graphicState{graphicSet.numericID};
    world.addGraphicsComponents(newEntity, graphicState);

    // Set the default client entity collision.
    registry.patch<CollisionBitSets>(
        newEntity, [&](CollisionBitSets& collisionBitSets) {
            collisionBitSets.setCollisionLayers(
                CollisionLayerType::ClientEntity, newEntity, registry);
            collisionBitSets.setCollisionMask(
                CollisionLayerType::TerrainWall
                | CollisionLayerType::BlockCollision);
        });

    // Add the new client entity to the network ID map.
    world.netIDMap[clientConnected.clientID] = newEntity;

    LOG_INFO("Constructed client entity with netID: %u, accountID: %lld, "
             "entityID: %u",
             clientConnected.clientID,
             static_cast<long long>(clientConnected.accountID), newEntity);

    // Build and send the response.
    sendConnectionResponse(clientConnected.clientID, newEntity);

    return newEntity;
}

void ClientConnectionSystem::attachClientToEntity(
    const ClientConnected& clientConnected, entt::entity entity)
{
    entt::registry& registry{world.registry};

    // If the entity is still attached to a client, detach it.
    // Note: This shouldn't happen, since the Network disconnects an account's
    //       old client before connecting the new one.
    NetworkID oldNetID{registry.get<ClientSimData>(entity).netID};
    if (oldNetID != NULL_NETWORK_ID) {
        LOG_ERROR("Account's entity is still attached to a client. "
                  "accountID: %lld, old netID: %u",
                  static_cast<long long>(clientConnected.accountID), oldNetID);
        world.netIDMap.erase(oldNetID);
    }

    // Stop the entity from being removed.
    removalTickMap.erase(clientConnected.accountID);

    // Re-construct the client data. This gives the new client a fresh AOI
    // list, and causes the systems that observe ClientSimData to send it its
    // initial state (inventory, self components, etc).
    registry.erase<ClientSimData>(entity);
    registry.emplace<ClientSimData>(entity, clientConnected.clientID,
                                    clientConnected.accountID);

    world.netIDMap[clientConnected.clientID] = entity;

    LOG_INFO("Attached client to existing entity. netID: %u, accountID: %lld, "
             "entityID: %u",
             clientConnected.clientID,
             static_cast<long long>(clientConnected.accountID), entity);

    // Build and send the response.
    sendConnectionResponse(clientConnected.clientID, entity);
}

void ClientConnectionSystem::removeExpiredEntities()
{
    Uint32 currentTick{simulation.getCurrentTick()};
    for (auto it = removalTickMap.begin(); it != removalTickMap.end();) {
        if (currentTick >= it->second) {
            removeClientEntity(it->first);
            it = removalTickMap.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ClientConnectionSystem::removeClientEntity(Sint64 accountID)
{
    auto entityIt{accountEntityMap.find(accountID)};
    if (entityIt == accountEntityMap.end()) {
        LOG_ERROR("Failed to find entity for accountID: %lld while removing.",
                  static_cast<long long>(accountID));
        return;
    }

    // Destroy the entity (if something else hasn't already).
    // Note: This will cause it to be removed from the entity locator,
    //       triggering ClientAOISystem to tell peers to delete it.
    entt::entity entity{entityIt->second};
    if (world.registry.valid(entity)) {
        world.registry.destroy(entity);
        LOG_INFO("Removed entity with entityID: %u", entity);
    }

    accountEntityMap.erase(entityIt);
}

void ClientConnectionSystem::sendConnectionResponse(NetworkID networkID,
                                                    entt::entity newEntity)
{
    // Fill in the current tick and their entity's ID.
    ConnectionResponse connectionResponse{};
    Uint32 currentTick{simulation.getCurrentTick()};
    connectionResponse.result = ConnectionResponse::Success;
    connectionResponse.entity = newEntity;
    connectionResponse.tickNum = currentTick;

    // Fill in the map's size.
    const ChunkExtent& mapChunkExtent{world.tileMap.getChunkExtent()};
    connectionResponse.mapXLengthChunks = mapChunkExtent.xLength;
    connectionResponse.mapYLengthChunks = mapChunkExtent.yLength;
    connectionResponse.mapZLengthChunks = mapChunkExtent.zLength;

    // Send the connection response message.
    network.clientEndpoint.send(networkID, connectionResponse, currentTick);
}

} // namespace WorldServer
} // namespace AM
