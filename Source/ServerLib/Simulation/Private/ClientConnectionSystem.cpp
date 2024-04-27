#include "ClientConnectionSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "GraphicData.h"
#include "SharedConfig.h"
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
namespace Server
{
ClientConnectionSystem::ClientConnectionSystem(Simulation& inSimulation,
                                               World& inWorld,
                                               Network& inNetwork,
                                               GraphicData& inGraphicData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, graphicData{inGraphicData}
, clientConnectionEventQueue{network.getEventDispatcher()}
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
}

void ClientConnectionSystem::processConnectEvent(
    const ClientConnected& clientConnected)
{
    entt::registry& registry{world.registry};

    // Create the entity and construct its standard components.
    // Note: Be careful with holding onto references here. If components
    //       are added to the same group, the ref will be invalidated.
    entt::entity newEntity{world.createEntity(world.getSpawnPoint())};

    registry.emplace<IsClientEntity>(newEntity);
    registry.emplace<Name>(
        newEntity, "Player " + std::to_string(static_cast<Uint32>(newEntity)));
    registry.emplace<Inventory>(newEntity);

    registry.emplace<ClientSimData>(newEntity, clientConnected.clientID,
                                    std::vector<entt::entity>());

    world.addMovementComponents(newEntity, Rotation{});

    const EntityGraphicSet& graphicSet{graphicData.getEntityGraphicSet(
        SharedConfig::DEFAULT_ENTITY_GRAPHIC_SET)};
    GraphicState graphicState{graphicSet.numericID};
    world.addGraphicsComponents(newEntity, graphicState);

    // Add the new client entity to the network ID map.
    world.netIDMap[clientConnected.clientID] = newEntity;

    LOG_INFO("Constructed client entity with netID: %u, entityID: %u",
             clientConnected.clientID, newEntity);

    // Build and send the response.
    sendConnectionResponse(clientConnected.clientID, newEntity);
}

void ClientConnectionSystem::processDisconnectEvent(
    const ClientDisconnected& clientDisconnected)
{
    // Find the disconnected client's associated entity.
    auto disconnectedEntityIt{
        world.netIDMap.find(clientDisconnected.clientID)};
    if (disconnectedEntityIt != world.netIDMap.end()) {
        // Found the entity. Remove it from the entity locator.
        // Note: Since the entity was removed from the locator, its peers
        //       will be told by ClientAOISystem to delete it.
        entt::entity disconnectedEntity{disconnectedEntityIt->second};
        world.entityLocator.removeEntity(disconnectedEntity);

        // Remove it from the registry and network ID map.
        world.registry.destroy(disconnectedEntity);
        world.netIDMap.erase(disconnectedEntityIt);

        LOG_INFO("Removed entity with entityID: %u", disconnectedEntity);
    }
    else {
        LOG_ERROR("Failed to find entity with netID: %u while erasing.",
                  clientDisconnected.clientID);
    }
}

void ClientConnectionSystem::sendConnectionResponse(NetworkID networkID,
                                                    entt::entity newEntity)
{
    // Fill in the current tick and their entity's ID.
    ConnectionResponse connectionResponse{};
    Uint32 currentTick{simulation.getCurrentTick()};
    connectionResponse.entity = newEntity;
    connectionResponse.tickNum = currentTick;

    // Fill in the map's size.
    const ChunkExtent& mapChunkExtent{world.tileMap.getChunkExtent()};
    connectionResponse.mapXLengthChunks = mapChunkExtent.xLength;
    connectionResponse.mapYLengthChunks = mapChunkExtent.yLength;

    // Send the connection response message.
    network.serializeAndSend(networkID, connectionResponse, currentTick);
}

} // namespace Server
} // namespace AM
