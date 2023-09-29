#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "DispatchMessage.h"
#include "IMessageProcessorExtension.h"
#include "ServerNetworkDefs.h"
#include "Heartbeat.h"
#include "InputChangeRequest.h"
#include "ChunkUpdateRequest.h"
#include "EntityInitRequest.h"
#include "EntityDeleteRequest.h"
#include "NameChangeRequest.h"
#include "AnimationStateChangeRequest.h"
#include "InitScriptRequest.h"
#include "InteractionRequest.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "EntityDelete.h"
#include "Log.h"

namespace AM
{
namespace Server
{
MessageProcessor::MessageProcessor(EventDispatcher& inNetworkEventDispatcher)
: networkEventDispatcher{inNetworkEventDispatcher}
{
}

Sint64 MessageProcessor::processReceivedMessage(NetworkID netID,
                                                Uint8 messageType,
                                                Uint8* messageBuffer,
                                                std::size_t messageSize)
{
    // The tick that the received message corresponds to.
    // Will be -1 if the message doesn't correspond to any tick.
    Sint64 messageTick{-1};

    // Match the enum values to their event types.
    EngineMessageType engineMessageType{
        static_cast<EngineMessageType>(messageType)};
    switch (engineMessageType) {
        case EngineMessageType::Heartbeat: {
            messageTick = static_cast<Sint64>(
                handleHeartbeat(messageBuffer, messageSize));
            break;
        }
        case EngineMessageType::InputChangeRequest: {
            messageTick = static_cast<Sint64>(
                handleInputChangeRequest(netID, messageBuffer, messageSize));
            break;
        }
        case EngineMessageType::ChunkUpdateRequest: {
            handleChunkUpdateRequest(netID, messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::EntityInitRequest: {
            dispatchMessage<EntityInitRequest>(
                messageBuffer, messageSize, networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityDeleteRequest: {
            dispatchMessage<EntityDeleteRequest>(
                messageBuffer, messageSize, networkEventDispatcher);
            break;
        }
        case EngineMessageType::NameChangeRequest: {
            dispatchMessage<NameChangeRequest>(messageBuffer, messageSize,
                                               networkEventDispatcher);
            break;
        }
        case EngineMessageType::AnimationStateChangeRequest: {
            dispatchMessage<AnimationStateChangeRequest>(
                messageBuffer, messageSize, networkEventDispatcher);
            break;
        }
        case EngineMessageType::InitScriptRequest: {
            handleInitScriptRequest(netID, messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::InteractionRequest: {
            handleInteractionRequest(netID, messageBuffer, messageSize);
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
                extension->processReceivedMessage(netID, messageType,
                                                  messageBuffer, messageSize);
            }
            break;
        }
    }

    return messageTick;
}

void MessageProcessor::setExtension(
    std::unique_ptr<IMessageProcessorExtension> inExtension)
{
    extension = std::move(inExtension);
}

Uint32 MessageProcessor::handleHeartbeat(Uint8* messageBuffer,
                                         std::size_t messageSize)
{
    // Deserialize the message.
    Heartbeat heartbeat{};
    Deserialize::fromBuffer(messageBuffer, messageSize, heartbeat);

    // Return the tick number associated with this message.
    return heartbeat.tickNum;
}

Uint32 MessageProcessor::handleInputChangeRequest(NetworkID netID,
                                                  Uint8* messageBuffer,
                                                  std::size_t messageSize)
{
    // Deserialize the message.
    InputChangeRequest inputChangeRequest{};
    Deserialize::fromBuffer(messageBuffer, messageSize, inputChangeRequest);

    // Fill in the network ID that we assigned to this client.
    inputChangeRequest.netID = netID;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<InputChangeRequest>(inputChangeRequest);

    // Return the tick number associated with this message.
    return inputChangeRequest.tickNum;
}

void MessageProcessor::handleChunkUpdateRequest(NetworkID netID,
                                                Uint8* messageBuffer,
                                                std::size_t messageSize)
{
    // Deserialize the message.
    ChunkUpdateRequest chunkUpdateRequest{};
    Deserialize::fromBuffer(messageBuffer, messageSize, chunkUpdateRequest);

    // Fill in the network ID that we assigned to this client.
    chunkUpdateRequest.netID = netID;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<ChunkUpdateRequest>(chunkUpdateRequest);
}

void MessageProcessor::handleInitScriptRequest(NetworkID netID,
                                               Uint8* messageBuffer,
                                               std::size_t messageSize)
{
    // Deserialize the message.
    InitScriptRequest initScriptRequest{};
    Deserialize::fromBuffer(messageBuffer, messageSize, initScriptRequest);

    // Fill in the network ID that we assigned to this client.
    initScriptRequest.netID = netID;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<InitScriptRequest>(initScriptRequest);
}

void MessageProcessor::handleInteractionRequest(NetworkID netID,
                                               Uint8* messageBuffer,
                                               std::size_t messageSize)
{
    // Deserialize the message.
    InteractionRequest interactionRequest{};
    Deserialize::fromBuffer(messageBuffer, messageSize, interactionRequest);

    // Fill in the network ID that we assigned to this client.
    interactionRequest.netID = netID;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<InteractionRequest>(interactionRequest);
}

} // End namespace Server
} // End namespace AM
