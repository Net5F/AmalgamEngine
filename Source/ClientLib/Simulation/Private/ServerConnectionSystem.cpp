#include "ServerConnectionSystem.h"
#include "World.h"
#include "Network.h"
#include "GraphicData.h"
#include "Name.h"
#include "Inventory.h"
#include "PreviousPosition.h"
#include "Position.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "Collision.h"
#include "InputHistory.h"
#include "Rotation.h"
#include "ClientGraphicState.h"
#include "GraphicState.h"
#include "IsClientEntity.h"
#include "UserConfig.h"
#include "Camera.h"
#include "SDLHelpers.h"
#include "NeedsAdjacentChunks.h"
#include "Transforms.h"
#include "Config.h"
#include "Log.h"

namespace AM
{
namespace Client
{
ServerConnectionSystem::ServerConnectionSystem(
    World& inWorld, EventDispatcher& inUiEventDispatcher, Network& inNetwork,
    GraphicData& inGraphicData, std::atomic<Uint32>& inCurrentTick)
: world{inWorld}
, network{inNetwork}
, graphicData{inGraphicData}
, currentTick{inCurrentTick}
, connectionRequestQueue{inUiEventDispatcher}
, connectionResponseQueue{network.getEventDispatcher()}
, connectionErrorQueue{network.getEventDispatcher()}
, connectionState{ConnectionState::Disconnected}
, connectionAttemptTimer{}
, simulationStartedSig{}
, serverConnectionErrorSig{}
, simulationStarted{simulationStartedSig}
, serverConnectionError{serverConnectionErrorSig}
{
}

void ServerConnectionSystem::processConnectionEvents()
{
    if (connectionState == ConnectionState::Disconnected) {
        // Check for a connection request from the UI.
        ConnectionRequest connectionRequest;
        if (connectionRequestQueue.pop(connectionRequest)) {
            if (Config::RUN_OFFLINE) {
                // No need to connect if we're running offline. Just mock up
                // the player data.
                initMockSimState();
                return;
            }
            else {
                // Kick off a connection attempt with the server.
                // Note: Eventually we'll instead send a ConnectionRequest to
                //       the login server here with our login info.
                network.connect();
                connectionState = ConnectionState::AwaitingResponse;
                connectionAttemptTimer.reset();
            }
        }
    }
    else if (connectionState == ConnectionState::AwaitingResponse) {
        // Wait for a connection response from the server.
        ConnectionResponse connectionResponse;
        if (connectionResponseQueue.pop(connectionResponse)) {
            initSimState(connectionResponse);
            connectionState = ConnectionState::Connected;
            simulationStartedSig.publish();
        }

        // If we've timed out, send a failure signal.
        if (connectionAttemptTimer.getTime() >= CONNECTION_RESPONSE_WAIT_S) {
            serverConnectionErrorSig.publish({ConnectionError::Type::Failed});
            connectionState = ConnectionState::Disconnected;
        }
    }

    // If the connection is lost, reset all network and sim state.
    ConnectionError connectionError;
    if (connectionErrorQueue.pop(connectionError)) {
        serverConnectionErrorSig.publish(connectionError);
        network.disconnect();
        clearSimState();
        connectionState = ConnectionState::Disconnected;
    }
}

ServerConnectionSystem::ConnectionState
    ServerConnectionSystem::getConnectionState()
{
    return connectionState;
}

void ServerConnectionSystem::initSimState(
    ConnectionResponse& connectionResponse)
{
    LOG_INFO("Received connection response. ID: %u, tick: %u",
             connectionResponse.entity, connectionResponse.tickNum);

    // Resize the world's tile map.
    world.tileMap.setMapSize(connectionResponse.mapXLengthChunks,
                             connectionResponse.mapYLengthChunks,
                             connectionResponse.mapZLengthChunks);
    LOG_INFO("Setting map size to: (%u, %u, %u)ch.",
             connectionResponse.mapXLengthChunks,
             connectionResponse.mapYLengthChunks,
             connectionResponse.mapZLengthChunks);

    // Allocate the locator grids.
    world.entityLocator.setGridSize(world.tileMap.getTileExtent());
    world.collisionLocator.setGridSize(world.tileMap.getTileExtent());

    // Aim our tick for some reasonable point ahead of the server.
    // The server will adjust us after the first message anyway.
    currentTick = connectionResponse.tickNum + Config::INITIAL_TICK_OFFSET;

    // Save the player entity ID so we can treat it differently.
    world.playerEntity = connectionResponse.entity;
}

void ServerConnectionSystem::initMockSimState()
{
    // Create the player entity.
    entt::registry& registry{world.registry};
    entt::entity newEntity{registry.create()};

    // Save the player entity ID for convenience.
    world.playerEntity = newEntity;

    // Set up the player's sim components.
    // Note: Be careful with holding onto references here. If components
    //       are added to the same group, the ref will be invalidated.
    registry.emplace<IsClientEntity>(newEntity);
    registry.emplace<Name>(newEntity,
                           std::to_string(static_cast<Uint32>(newEntity)));
    registry.emplace<Inventory>(newEntity);

    registry.emplace<Input>(newEntity);
    registry.emplace<Position>(newEntity, 0.0f, 0.0f, 0.0f);
    world.entityLocator.updateEntity(newEntity, {0.0f, 0.0f, 0.0f});
    registry.emplace<PreviousPosition>(newEntity, 0.0f, 0.0f, 0.0f);
    registry.emplace<Movement>(newEntity);
    registry.emplace<MovementModifiers>(newEntity);
    registry.emplace<Rotation>(newEntity);
    registry.emplace<InputHistory>(newEntity);

    const EntityGraphicSet& graphicSet{graphicData.getEntityGraphicSet(
        SharedConfig::DEFAULT_ENTITY_GRAPHIC_SET)};
    registry.emplace<GraphicState>(newEntity, graphicSet.numericID);

    // Entity collision always comes from its Idle South graphic.
    const auto& graphicArr{graphicSet.graphics.at(EntityGraphicType::Idle)};
    const GraphicRef& graphic{graphicArr.at(Rotation::Direction::South)};
    const BoundingBox& modelBounds{graphic.getModelBounds()};
    const Collision& collision{registry.emplace<Collision>(
        newEntity, modelBounds,
        Transforms::modelToWorldEntity(modelBounds,
                                       registry.get<Position>(newEntity)))};

    // Entities with Collision get added to the locator.
    CollisionObjectType::Value objectType{
        world.registry.all_of<IsClientEntity>(newEntity)
            ? CollisionObjectType::ClientEntity
            : CollisionObjectType::NonClientEntity};
    world.collisionLocator.updateEntity(newEntity, collision.worldBounds,
                                        objectType);

    // Entities with GraphicState also get a ClientGraphicState.
    registry.emplace<ClientGraphicState>(newEntity, EntityGraphicType::Idle,
                                         Rotation::Direction::South);

    // TODO: Switch to logical screen size and do scaling in Renderer.
    UserConfig& userConfig{UserConfig::get()};
    registry.emplace<Camera>(
        newEntity, Camera::CenterOnEntity, Position{}, PreviousPosition{},
        BoundingBox{}, SDLHelpers::rectToFRect(userConfig.getWindowSize()));
}

void ServerConnectionSystem::clearSimState()
{
    world.registry.clear();
    world.playerEntity = entt::null;
    world.tileMap.clear();

    // The current tick needs to be set back to 0 so the Network knows to stop
    // sending heartbeats.
    currentTick = 0;
}

} // namespace Client
} // namespace AM
