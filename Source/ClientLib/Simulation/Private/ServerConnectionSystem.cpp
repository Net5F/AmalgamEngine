#include "ServerConnectionSystem.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "Name.h"
#include "PreviousPosition.h"
#include "Position.h"
#include "Collision.h"
#include "InputHistory.h"
#include "Rotation.h"
#include "AnimationState.h"
#include "IsClientEntity.h"
#include "UserConfig.h"
#include "Camera.h"
#include "SDLHelpers.h"
#include "NeedsAdjacentChunks.h"
#include "Transforms.h"
#include "Config.h"
#include "Ignore.h"
#include "Log.h"

namespace AM
{
namespace Client
{
ServerConnectionSystem::ServerConnectionSystem(
    World& inWorld, EventDispatcher& inUiEventDispatcher, Network& inNetwork,
    SpriteData& inSpriteData, std::atomic<Uint32>& inCurrentTick)
: world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, currentTick{inCurrentTick}
, connectionRequestQueue{inUiEventDispatcher}
, connectionResponseQueue{network.getEventDispatcher()}
, connectionErrorQueue{network.getEventDispatcher()}
, connectionState{ConnectionState::Disconnected}
, connectionAttemptTimer{}
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
            serverConnectionErrorSig.publish(
                {ConnectionError::Type::Failed});
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
                             connectionResponse.mapYLengthChunks);
    LOG_INFO("Setting map size to: (%u, %u)ch.",
             connectionResponse.mapXLengthChunks,
             connectionResponse.mapYLengthChunks);

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

    registry.emplace<Input>(newEntity);
    registry.emplace<Position>(newEntity, 0.0f, 0.0f, 0.0f);
    registry.emplace<PreviousPosition>(newEntity, 0.0f, 0.0f, 0.0f);
    registry.emplace<Rotation>(newEntity);
    registry.emplace<InputHistory>(newEntity);

    // TODO: When we add character sprite sets, update this.
    Uint16 spriteSetID{spriteData
            .getObjectSpriteSet(SharedConfig::DEFAULT_CHARACTER_SPRITE_SET)
            .numericID};
    const auto& animationState{registry.emplace<AnimationState>(
        newEntity, SpriteSet::Type::Object, spriteSetID,
        SharedConfig::DEFAULT_CHARACTER_SPRITE_INDEX)};
    const Sprite* sprite{
        spriteData.getObjectSpriteSet(animationState.spriteSetID)
            .sprites[animationState.spriteIndex]};
    registry.emplace<Sprite>(newEntity, *sprite);

    registry.emplace<Collision>(
        newEntity, sprite->modelBounds,
        Transforms::modelToWorldCentered(sprite->modelBounds,
                                         registry.get<Position>(newEntity)));

    // TODO: Switch to logical screen size and do scaling in Renderer.
    UserConfig& userConfig{UserConfig::get()};
    registry.emplace<Camera>(
        newEntity, Camera::CenterOnEntity, Position{}, PreviousPosition{},
        SDLHelpers::rectToFRect(userConfig.getWindowSize()));
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
