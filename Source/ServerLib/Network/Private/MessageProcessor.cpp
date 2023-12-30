#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "DispatchMessage.h"
#include "IMessageProcessorExtension.h"
#include "ServerNetworkDefs.h"
#include "Heartbeat.h"
#include "InputChangeRequest.h"
#include "EntityNameChangeRequest.h"
#include "AnimationStateChangeRequest.h"
#include "ChunkDataRequest.h"
#include "EntityInitRequest.h"
#include "EntityDeleteRequest.h"
#include "EntityInitScriptRequest.h"
#include "EntityInteractionRequest.h"
#include "ItemDataRequest.h"
#include "ItemInitRequest.h"
#include "ItemChangeRequest.h"
#include "ItemInitScriptRequest.h"
#include "ItemInteractionRequest.h"
#include "CombineItemsRequest.h"
#include "UseItemOnEntityRequest.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "InventoryOperation.h"
#include "Log.h"
#include <span>

namespace AM
{
namespace Server
{
template<typename T>
void dispatchWithNetID(NetworkID netID, std::span<Uint8> messageBuffer,
                       EventDispatcher& dispatcher)
{
    // Deserialize the message.
    T message{};
    Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                            message);

    // Fill in the network ID that we assigned to this client.
    message.netID = netID;

    // Push the message into any subscribed queues.
    dispatcher.push<T>(message);
}

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
        case EngineMessageType::EntityNameChangeRequest: {
            dispatchMessage<EntityNameChangeRequest>(messageBuffer, messageSize,
                                                     networkEventDispatcher);
            break;
        }
        case EngineMessageType::AnimationStateChangeRequest: {
            dispatchMessage<AnimationStateChangeRequest>(
                messageBuffer, messageSize, networkEventDispatcher);
            break;
        }
        case EngineMessageType::ChunkDataRequest: {
            dispatchWithNetID<ChunkDataRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityInitRequest: {
            dispatchWithNetID<EntityInitRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityDeleteRequest: {
            dispatchMessage<EntityDeleteRequest>(messageBuffer, messageSize,
                                                 networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityInitScriptRequest: {
            dispatchWithNetID<EntityInitScriptRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityInteractionRequest: {
            dispatchWithNetID<EntityInteractionRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemDataRequest: {
            dispatchWithNetID<ItemDataRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemInitRequest: {
            dispatchWithNetID<ItemInitRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemChangeRequest: {
            dispatchWithNetID<ItemChangeRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemInitScriptRequest: {
            dispatchWithNetID<ItemInitScriptRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemInteractionRequest: {
            dispatchWithNetID<ItemInteractionRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::CombineItemsRequest: {
            dispatchWithNetID<CombineItemsRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::UseItemOnEntityRequest: {
            dispatchWithNetID<UseItemOnEntityRequest>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileAddLayer: {
            dispatchWithNetID<TileAddLayer>(netID, {messageBuffer, messageSize},
                                            networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileRemoveLayer: {
            dispatchWithNetID<TileRemoveLayer>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileClearLayers: {
            dispatchWithNetID<TileClearLayers>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileExtentClearLayers: {
            dispatchWithNetID<TileExtentClearLayers>(
                netID, {messageBuffer, messageSize}, networkEventDispatcher);
            break;
        }
        case EngineMessageType::InventoryOperation: {
            dispatchMessage<InventoryOperation>(messageBuffer, messageSize,
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

} // End namespace Server
} // End namespace AM
