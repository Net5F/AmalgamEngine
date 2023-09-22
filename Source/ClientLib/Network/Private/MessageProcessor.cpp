#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "DispatchMessage.h"
#include "IMessageProcessorExtension.h"
#include "ExplicitConfirmation.h"
#include "ConnectionResponse.h"
#include "UserErrorString.h"
#include "ChunkUpdate.h"
#include "EntityInit.h"
#include "ComponentUpdate.h"
#include "InitScriptResponse.h"
#include "EntityDelete.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "PlayerMovementUpdate.h"
#include "NpcMovementUpdate.h"
#include "ReplicatedComponentTools.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
namespace Client
{
MessageProcessor::MessageProcessor(EventDispatcher& inNetworkEventDispatcher)
: networkEventDispatcher{inNetworkEventDispatcher}
, playerEntity{entt::null}
, lastReceivedTick{0}
{
}

void MessageProcessor::processReceivedMessage(Uint8 messageType,
                                              Uint8* messageBuffer,
                                              std::size_t messageSize)
{
    // Match the enum values to their event types.
    EngineMessageType engineMessageType{
        static_cast<EngineMessageType>(messageType)};
    switch (engineMessageType) {
        case EngineMessageType::ExplicitConfirmation: {
            handleExplicitConfirmation(messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::ConnectionResponse: {
            handleConnectionResponse(messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::UserErrorString: {
            dispatchMessage<UserErrorString>(messageBuffer, messageSize,
                                             networkEventDispatcher);
            break;
        }
        case EngineMessageType::ChunkUpdate: {
            dispatchMessageSharedPtr<ChunkUpdate>(messageBuffer, messageSize,
                                                  networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityInit: {
            dispatchMessage<EntityInit>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::ComponentUpdate: {
            handleComponentUpdate(messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::InitScriptResponse: {
            dispatchMessage<InitScriptResponse>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileAddLayer: {
            dispatchMessage<TileAddLayer>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileRemoveLayer: {
            dispatchMessage<TileRemoveLayer>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileClearLayers: {
            dispatchMessage<TileClearLayers>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileExtentClearLayers: {
            dispatchMessage<TileExtentClearLayers>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityDelete: {
            dispatchMessage<EntityDelete>(messageBuffer, messageSize,
                                          networkEventDispatcher);
            break;
        }
        default: {
            // If we don't have a handler for this message type, pass it to
            // the project.
            if (extension != nullptr) {
                extension->processReceivedMessage(messageType, messageBuffer,
                                                  messageSize);
            }
            break;
        }
    }
}

Uint32 MessageProcessor::getLastReceivedTick()
{
    return lastReceivedTick;
}

void MessageProcessor::setExtension(
    std::unique_ptr<IMessageProcessorExtension> inExtension)
{
    extension = std::move(inExtension);
}

void MessageProcessor::handleExplicitConfirmation(Uint8* messageBuffer,
                                                  std::size_t messageSize)
{
    // Deserialize the message.
    ExplicitConfirmation explicitConfirmation{};
    Deserialize::fromBuffer(messageBuffer, messageSize, explicitConfirmation);

    // Move lastReceivedTick forward.
    lastReceivedTick += explicitConfirmation.confirmedTickCount;
}

void MessageProcessor::handleConnectionResponse(Uint8* messageBuffer,
                                                std::size_t messageSize)
{
    // Deserialize the message.
    ConnectionResponse connectionResponse{};
    Deserialize::fromBuffer(messageBuffer, messageSize, connectionResponse);

    // Save our player entity so we can determine which update messages are for
    // the player.
    playerEntity = connectionResponse.entity;

    // Initialize lastReceivedTick.
    lastReceivedTick = connectionResponse.tickNum;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<ConnectionResponse>(connectionResponse);
}

void MessageProcessor::handleComponentUpdate(Uint8* messageBuffer, std::size_t messageSize)
{
    // Deserialize the message.
    ComponentUpdate componentUpdate{};
    Deserialize::fromBuffer(messageBuffer, messageSize, componentUpdate);

    // If the message's tick is newer than our saved tick, update it.
    if (componentUpdate.tickNum > lastReceivedTick) {
        lastReceivedTick = componentUpdate.tickNum;
    }
    AM_ASSERT((componentUpdate.tickNum >= lastReceivedTick),
              "Received ticks out of order. last: %u, new: %u",
              lastReceivedTick.load(), componentUpdate.tickNum);

    // Give the interceptors a chance to grab components.
    interceptMovementUpdates(componentUpdate);

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<ComponentUpdate>(componentUpdate);
}

void MessageProcessor::interceptMovementUpdates(
    ComponentUpdate& componentUpdate)
{
    // If the update contains our desired components.
    if (ReplicatedComponentTools::containsTypes<Input, Position, Velocity,
                                                Rotation>(
            componentUpdate.components)) {
        // Find all of the desired components, push them into a new struct, 
        // and erase them from componentUpdate.
        PlayerMovementUpdate movementUpdate{componentUpdate.tickNum,
                                            componentUpdate.entity};
        auto& components{componentUpdate.components};
        for (auto it = components.begin(); it != components.end();) {
            if (std::visit(movementUpdate, *it)) {
                it = components.erase(it);
            }
            else {
                ++it;
            }
        }

        // If this update is for the player, push a PlayerMovementUpdate.
        if (componentUpdate.entity == playerEntity) {
            networkEventDispatcher.push(movementUpdate);
        }
        else {
            // Not for the player, push an NpcMovementUpdate.
            networkEventDispatcher.push<NpcMovementUpdate>(
                static_cast<NpcMovementUpdate>(movementUpdate));
        }
    }
}

} // End namespace Client
} // End namespace AM
