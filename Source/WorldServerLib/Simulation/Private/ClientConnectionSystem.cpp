#include "ClientConnectionSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "ISimulationExtension.h"
#include "ClientEntityLoadInfo.h"
#include "Network.h"
#include "GraphicData.h"
#include "Database.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "ClientPersistenceUsage.h"
#include "AccountSaveQueue.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Serialize.h"
#include "ConnectionResponse.h"
#include "WorldLogoutUpdate.h"
#include "DisconnectNotice.h"
#include "Input.h"
#include "Rotation.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "CastState.h"
#include "Name.h"
#include "Inventory.h"
#include "ClientSimData.h"
#include "IsClientEntity.h"
#include "GraphicState.h"
#include "CollisionBitSets.h"
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
, extension{nullptr}
, clientConnectionEventQueue{network.clientEndpoint.getEventDispatcher()}
, logoutRequestQueue{inSimContext.networkEventDispatcher}
, castStartedObserver{}
, accountEntityMap{}
, removalTickMap{}
, logoutTickMap{}
, rejectedClients{}
{
    castStartedObserver.bind(world.registry);
    castStartedObserver.on_construct<CastState>();
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

    // Start or cancel any requested logouts.
    WorldLogoutRequest logoutRequest{};
    while (logoutRequestQueue.pop(logoutRequest)) {
        processLogoutRequest(logoutRequest);
    }

    // Cancel or complete any ongoing logouts.
    updateLogouts();

    // Remove any entities that finished lingering.
    removeExpiredEntities();
}

void ClientConnectionSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
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
        entt::entity newEntity{createClientEntity(clientConnected)};
        if (newEntity != entt::null) {
            accountEntityMap[clientConnected.accountID] = newEntity;
        }
    }
}

void ClientConnectionSystem::processDisconnectEvent(
    const ClientDisconnected& clientDisconnected)
{
    // If we rejected this client, it has no entity. Nothing to do.
    if (rejectedClients.erase(clientDisconnected.clientHandle)) {
        return;
    }

    // Find the disconnected client's associated entity.
    auto disconnectedEntityIt{
        world.clientHandleMap.find(clientDisconnected.clientHandle)};
    if (disconnectedEntityIt == world.clientHandleMap.end()) {
        LOG_ERROR(
            "Failed to find entity with clientID: %u while disconnecting.",
            clientDisconnected.clientHandle.id());
        return;
    }

    // Detach the client from its entity.
    entt::entity entity{disconnectedEntityIt->second};
    world.clientHandleMap.erase(disconnectedEntityIt);
    ClientSimData& client{world.registry.get<ClientSimData>(entity)};
    client.clientHandle = NULL_CONNECTION_HANDLE;
    Sint64 accountID{client.accountID};

    // If the client was logging out, stop. The entity will linger as usual.
    logoutTickMap.erase(accountID);

    // If the client logged out (or entities don't linger), remove the entity
    // immediately.
    if ((clientDisconnected.reason == DisconnectNotice::Logout)
        || (Config::LOGOUT_DELAY_TICKS == 0)) {
        removeClientEntity(accountID);
        return;
    }

    // Release any inputs the client was holding, so the entity doesn't keep
    // moving while it waits to be removed.
    world.registry.replace<Input>(entity);

    // Start lingering.
    removalTickMap[accountID]
        = simulation.getCurrentTick() + Config::LOGOUT_DELAY_TICKS;
    LOG_INFO("Client disconnected, entity is lingering. clientID: %u, "
             "entityID: %u",
             clientDisconnected.clientHandle.id(), entity);
}

entt::entity ClientConnectionSystem::createClientEntity(
    const ClientConnected& clientConnected)
{
    entt::registry& registry{world.registry};
    Sint64 accountID{clientConnected.accountID};

    // Get the account's saved Account components. If we have data that the
    // account server hasn't confirmed saving yet (e.g. the account logged out
    // and back in quickly), it's newer than what the account server sent, so
    // we use it instead.
    // Note: If nothing uses Account persistence, both of these are empty.
    const AccountData* accountData{&(clientConnected.accountData)};
    bool hadAccountData{clientConnected.hasAccountData};
    if (const AccountData* unsavedData{
            world.accountSaveQueue.getUnsavedData(accountID)}) {
        accountData = unsavedData;
        hadAccountData = true;
    }

    // Get the account's saved World components.
    // Note: If this fails, we reject the client instead of giving it fresh
    //       data, since the next save would overwrite its real data.
    SaveHelper::ClientWorldData worldComponents{};
    GetSavedComponentsResult worldResult{
        getSavedWorldComponents(accountID, worldComponents)};
    if (worldResult == GetSavedComponentsResult::Error) {
        LOG_ERROR("Failed to get saved data for accountID: %lld",
                  static_cast<long long>(accountID));
        rejectClient(clientConnected.clientHandle);
        return entt::null;
    }

    // Create the entity and load its saved components.
    entt::entity newEntity{world.loadHelper.loadClientEntity(
        accountID,
        {accountData->engineComponents, accountData->projectComponents},
        {worldComponents.engineComponents, worldComponents.projectComponents})};
    if (newEntity == entt::null) {
        rejectClient(clientConnected.clientHandle);
        return entt::null;
    }

    // Add defaults for any engine components that weren't loaded.
    if (!addDefaultComponents(newEntity, clientConnected)) {
        LOG_ERROR("Failed to add default components to client entity for "
                  "accountID: %lld",
                  static_cast<long long>(accountID));
        registry.destroy(newEntity);
        rejectClient(clientConnected.clientHandle);
        return entt::null;
    }

    // Let the project add its defaults.
    if (extension) {
        extension->initClientEntity(
            newEntity,
            {.accountID{accountID},
             .hadAccountData{hadAccountData},
             .hadWorldData{worldResult == GetSavedComponentsResult::Success}});
    }

    // Mark the entity as a client entity.
    // Note: We do this last, so anything that reacts to these sees the
    //       entity's loaded state.
    registry.emplace<IsClientEntity>(newEntity);
    registry.emplace<ClientSimData>(newEntity, clientConnected.clientHandle,
                                    accountID);

    // Add the new client entity to the client handle map.
    world.clientHandleMap[clientConnected.clientHandle] = newEntity;

    LOG_INFO("Constructed client entity with clientID: %u, accountID: %lld, "
             "entityID: %u",
             clientConnected.clientHandle.id(),
             static_cast<long long>(clientConnected.accountID), newEntity);

    // Build and send the response.
    sendConnectionResponse(clientConnected.clientHandle, newEntity);

    return newEntity;
}

void ClientConnectionSystem::attachClientToEntity(
    const ClientConnected& clientConnected, entt::entity entity)
{
    entt::registry& registry{world.registry};

    // If the entity is still attached to a client, detach it.
    // Note: This shouldn't happen, since the Network disconnects an account's
    //       old client before connecting the new one.
    ConnectionHandle oldClientHandle{
        registry.get<ClientSimData>(entity).clientHandle};
    if (oldClientHandle != NULL_CONNECTION_HANDLE) {
        LOG_ERROR("Account's entity is still attached to a client. "
                  "accountID: %lld, old clientID: %u",
                  static_cast<long long>(clientConnected.accountID),
                  oldClientHandle.id());
        world.clientHandleMap.erase(oldClientHandle);
    }

    // Stop the entity from being removed.
    removalTickMap.erase(clientConnected.accountID);

    // Re-construct the client data. This gives the new client a fresh AOI
    // list, and causes the systems that observe ClientSimData to send it its
    // initial state (inventory, self components, etc).
    registry.erase<ClientSimData>(entity);
    registry.emplace<ClientSimData>(entity, clientConnected.clientHandle,
                                    clientConnected.accountID);

    world.clientHandleMap[clientConnected.clientHandle] = entity;

    LOG_INFO(
        "Attached client to existing entity. clientID: %u, accountID: %lld, "
        "entityID: %u",
        clientConnected.clientHandle.id(),
        static_cast<long long>(clientConnected.accountID), entity);

    // Build and send the response.
    sendConnectionResponse(clientConnected.clientHandle, entity);
}

void ClientConnectionSystem::processLogoutRequest(
    const WorldLogoutRequest& logoutRequest)
{
    // Find the entity ID of the client that sent this request.
    entt::entity clientEntity{
        world.getClientEntity(logoutRequest.clientHandle)};
    if (clientEntity == entt::null) {
        // Client doesn't exist (may have disconnected), skip this request.
        return;
    }
    Sint64 accountID{world.registry.get<ClientSimData>(clientEntity).accountID};

    if (logoutRequest.type == WorldLogoutRequest::Start) {
        // If this client is already logging out, ignore the request.
        if (logoutTickMap.contains(accountID)) {
            return;
        }

        // Entities can't log out while moving.
        if (entityIsMoving(clientEntity)) {
            network.clientEndpoint.send(
                logoutRequest.clientHandle,
                WorldLogoutUpdate{WorldLogoutUpdate::Canceled});
            return;
        }

        // Start the countdown.
        // Note: If the delay is 0, updateLogouts() will complete it this tick.
        logoutTickMap[accountID]
            = simulation.getCurrentTick() + Config::LOGOUT_DELAY_TICKS;
        network.clientEndpoint.send(
            logoutRequest.clientHandle,
            WorldLogoutUpdate{WorldLogoutUpdate::Started,
                              static_cast<float>(Config::LOGOUT_DELAY_S)});
    }
    else if (logoutRequest.type == WorldLogoutRequest::Cancel) {
        if (logoutTickMap.contains(accountID)) {
            cancelLogout(accountID, clientEntity);
        }
    }
}

void ClientConnectionSystem::updateLogouts()
{
    entt::registry& registry{world.registry};

    // Cancel the logouts of any entities that started casting.
    for (entt::entity entity : castStartedObserver) {
        if (ClientSimData * client{registry.try_get<ClientSimData>(entity)}) {
            if (logoutTickMap.contains(client->accountID)) {
                cancelLogout(client->accountID, entity);
            }
        }
    }
    castStartedObserver.clear();

    // Cancel the logouts of any entities that moved, and complete any that
    // finished counting down.
    Uint32 currentTick{simulation.getCurrentTick()};
    for (auto it = logoutTickMap.begin(); it != logoutTickMap.end();) {
        auto [accountID, logoutTick] = *it;
        ++it;

        auto entityIt{accountEntityMap.find(accountID)};
        if ((entityIt == accountEntityMap.end())
            || !(registry.valid(entityIt->second))) {
            LOG_ERROR("Failed to find entity for accountID: %lld while "
                      "logging out.",
                      static_cast<long long>(accountID));
            logoutTickMap.erase(accountID);
            continue;
        }

        entt::entity entity{entityIt->second};
        if (entityIsMoving(entity)) {
            cancelLogout(accountID, entity);
        }
        else if (currentTick >= logoutTick) {
            completeLogout(accountID, entity);
        }
    }
}

void ClientConnectionSystem::cancelLogout(Sint64 accountID, entt::entity entity)
{
    logoutTickMap.erase(accountID);

    ConnectionHandle clientHandle{
        world.registry.get<ClientSimData>(entity).clientHandle};
    network.clientEndpoint.send(clientHandle,
                                WorldLogoutUpdate{WorldLogoutUpdate::Canceled});
}

void ClientConnectionSystem::completeLogout(Sint64 accountID,
                                            entt::entity entity)
{
    logoutTickMap.erase(accountID);

    // Tell the client that it's logged out, then disconnect it.
    // Note: When the disconnect event arrives, we'll remove the entity
    //       immediately (skipping the linger time).
    ConnectionHandle clientHandle{
        world.registry.get<ClientSimData>(entity).clientHandle};
    network.clientEndpoint.send(
        clientHandle, WorldLogoutUpdate{WorldLogoutUpdate::Completed});
    network.clientEndpoint.disconnect(clientHandle, DisconnectNotice::Logout);
    LOG_INFO("Client logged out. clientID: %u, accountID: %lld",
             clientHandle.id(), static_cast<long long>(accountID));
}

bool ClientConnectionSystem::entityIsMoving(entt::entity entity)
{
    entt::registry& registry{world.registry};

    // If the entity is trying to move, it counts as moving (even if it's
    // blocked).
    if (registry.get<Input>(entity).inputStates.any()) {
        return true;
    }

    // If the entity's position changed last tick (e.g. it was pushed or is
    // falling), it's moving.
    if (const PreviousPosition* previousPosition{
            registry.try_get<PreviousPosition>(entity)}) {
        if (registry.get<Position>(entity) != *previousPosition) {
            return true;
        }
    }

    return false;
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

    // Save and destroy the entity (if something else hasn't already
    // destroyed it).
    // Note: Destroying will cause it to be removed from the entity locator,
    //       triggering ClientAOISystem to tell peers to delete it.
    entt::entity entity{entityIt->second};
    if (world.registry.valid(entity)) {
        world.saveHelper.saveClientEntity(entity);
        world.registry.destroy(entity);
        LOG_INFO("Removed entity with entityID: %u", entity);
    }

    accountEntityMap.erase(entityIt);
}

void ClientConnectionSystem::sendConnectionResponse(
    ConnectionHandle clientHandle, entt::entity newEntity)
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
    network.clientEndpoint.send(clientHandle, connectionResponse, currentTick);
}

bool ClientConnectionSystem::addDefaultComponents(
    entt::entity entity, const ClientConnected& clientConnected)
{
    // Note: Be careful with holding onto references here. If components
    //       are added to the same group, the ref will be invalidated.
    entt::registry& registry{world.registry};

    if (!registry.all_of<Name>(entity)) {
        registry.emplace<Name>(entity, clientConnected.username);
    }
    if (!registry.all_of<Inventory>(entity)) {
        registry.emplace<Inventory>(entity);
    }
    if (!registry.all_of<Input>(entity)) {
        world.addMovementComponents(entity);
    }
    if (!registry.all_of<GraphicState>(entity)) {
        const EntityGraphicSet& graphicSet{graphicData.getEntityGraphicSet(
            SharedConfig::DEFAULT_ENTITY_GRAPHIC_SET)};
        GraphicState graphicState{graphicSet.numericID};
        if (!world.addGraphicsComponents(entity, graphicState)) {
            return false;
        }
    }

    // Set the default client entity collision (unless it's persisted).
    if constexpr (EnginePersistedComponentTypes::EntryFor<
                      CollisionBitSets>::CLIENT_PERSISTENCE
                  == ClientPersistence::None) {
        registry.patch<CollisionBitSets>(
            entity, [&](CollisionBitSets& collisionBitSets) {
                collisionBitSets.setCollisionLayers(
                    CollisionLayerType::ClientEntity, entity, registry);
                collisionBitSets.setCollisionMask(
                    CollisionLayerType::TerrainWall
                    | CollisionLayerType::BlockCollision);
            });
    }

    return true;
}

ClientConnectionSystem::GetSavedComponentsResult
    ClientConnectionSystem::getSavedWorldComponents(
        Sint64 accountID, SaveHelper::ClientWorldData& savedComponents)
{
    // If no components use World persistence, there's nothing to get.
    if constexpr (!USES_CLIENT_PERSISTENCE<ClientPersistence::World>) {
        return GetSavedComponentsResult::NotFound;
    }

    // If this account's last save failed, use its unsaved data.
    if (const SaveHelper::ClientWorldData* unsavedData{
            world.saveHelper.getUnsavedWorldData(accountID)}) {
        savedComponents = *unsavedData;
        return GetSavedComponentsResult::Success;
    }

    // Copy the saved data out of the database.
    auto copyComponents = [&](std::span<const Uint8> engineComponents,
                              std::span<const Uint8> projectComponents) {
        savedComponents.engineComponents.assign(engineComponents.begin(),
                                                engineComponents.end());
        savedComponents.projectComponents.assign(projectComponents.begin(),
                                                 projectComponents.end());
    };
    switch (world.database->getClientEntityData(accountID,
                                                std::move(copyComponents))) {
        case Database::GetClientEntityDataResult::Success: {
            return GetSavedComponentsResult::Success;
        }
        case Database::GetClientEntityDataResult::NotFound: {
            return GetSavedComponentsResult::NotFound;
        }
        default: {
            return GetSavedComponentsResult::Error;
        }
    }
}

void ClientConnectionSystem::rejectClient(ConnectionHandle clientHandle)
{
    // Tell the client that it failed to connect, then disconnect it.
    // Note: The client stops waiting for a connection when it receives the
    //       response, so the disconnect reason doesn't matter.
    ConnectionResponse connectionResponse{};
    connectionResponse.result = ConnectionResponse::InternalError;
    network.clientEndpoint.send(clientHandle, connectionResponse,
                                simulation.getCurrentTick());
    network.clientEndpoint.disconnect(clientHandle, DisconnectNotice::Kicked);

    // Ignore the disconnect event when it arrives, since there's no entity.
    rejectedClients.insert(clientHandle);
    LOG_INFO("Rejected client. clientID: %u", clientHandle.id());
}

} // namespace WorldServer
} // namespace AM
