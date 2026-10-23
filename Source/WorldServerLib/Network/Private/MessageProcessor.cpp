#include "MessageProcessor.h"
#include "MessageProcessorContext.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "DispatchMessage.h"
#include "IMessageProcessorExtension.h"
#include "Heartbeat.h"
#include "InputChangeRequest.h"
#include "EntityNameChangeRequest.h"
#include "GraphicStateChangeRequest.h"
#include "ChunkDataRequest.h"
#include "EntityInitRequest.h"
#include "EntityDeleteRequest.h"
#include "EntityInitScriptRequest.h"
#include "ItemDataRequest.h"
#include "ItemInitRequest.h"
#include "ItemChangeRequest.h"
#include "ItemInitScriptRequest.h"
#include "CombineItemsRequest.h"
#include "UseItemOnEntityRequest.h"
#include "DialogueChoiceRequest.h"
#include "CastRequest.h"
#include "WorldLogoutRequest.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "InventoryOperation.h"
#include "Log.h"
#include <span>

namespace AM
{
namespace WorldServer
{
template<typename T>
void dispatchWithClientHandle(ConnectionHandle clientHandle,
                              std::span<Uint8> messageBuffer,
                              EventDispatcher& dispatcher)
{
    // Deserialize the message.
    T message{};
    Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                            message);

    // Fill in the handle of the client that sent the message.
    message.clientHandle = clientHandle;

    // Push the message into any subscribed queues.
    dispatcher.push<T>(message);
}

MessageProcessor::MessageProcessor(
    const MessageProcessorContext& inMessageProcessorContext)
: networkEventDispatcher{inMessageProcessorContext.networkEventDispatcher}
{
}

Sint64 MessageProcessor::processReceivedMessage(ConnectionHandle clientHandle,
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
            messageTick = static_cast<Sint64>(handleInputChangeRequest(
                clientHandle, messageBuffer, messageSize));
            break;
        }
        case EngineMessageType::EntityNameChangeRequest: {
            dispatchMessage<EntityNameChangeRequest>(messageBuffer, messageSize,
                                                     networkEventDispatcher);
            break;
        }
        case EngineMessageType::GraphicStateChangeRequest: {
            dispatchMessage<GraphicStateChangeRequest>(
                messageBuffer, messageSize, networkEventDispatcher);
            break;
        }
        case EngineMessageType::ChunkDataRequest: {
            dispatchWithClientHandle<ChunkDataRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityInitRequest: {
            dispatchWithClientHandle<EntityInitRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityDeleteRequest: {
            dispatchMessage<EntityDeleteRequest>(messageBuffer, messageSize,
                                                 networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityInitScriptRequest: {
            dispatchWithClientHandle<EntityInitScriptRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemDataRequest: {
            dispatchWithClientHandle<ItemDataRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemInitRequest: {
            dispatchWithClientHandle<ItemInitRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemChangeRequest: {
            dispatchWithClientHandle<ItemChangeRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemInitScriptRequest: {
            dispatchWithClientHandle<ItemInitScriptRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::CombineItemsRequest: {
            dispatchWithClientHandle<CombineItemsRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::UseItemOnEntityRequest: {
            dispatchWithClientHandle<UseItemOnEntityRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::DialogueChoiceRequest: {
            dispatchWithClientHandle<DialogueChoiceRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::CastRequest: {
            dispatchWithClientHandle<CastRequest>(clientHandle,
                                                  {messageBuffer, messageSize},
                                                  networkEventDispatcher);
            break;
        }
        case EngineMessageType::WorldLogoutRequest: {
            dispatchWithClientHandle<WorldLogoutRequest>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileAddLayer: {
            dispatchWithClientHandle<TileAddLayer>(clientHandle,
                                                   {messageBuffer, messageSize},
                                                   networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileRemoveLayer: {
            dispatchWithClientHandle<TileRemoveLayer>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileClearLayers: {
            dispatchWithClientHandle<TileClearLayers>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileExtentClearLayers: {
            dispatchWithClientHandle<TileExtentClearLayers>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        case EngineMessageType::InventoryOperation: {
            dispatchWithClientHandle<InventoryOperation>(
                clientHandle, {messageBuffer, messageSize},
                networkEventDispatcher);
            break;
        }
        default: {
            // If we don't have a handler for this message type, pass it to
            // the project.
            if (extension != nullptr) {
                extension->processReceivedMessage(clientHandle, messageType,
                                                  messageBuffer, messageSize);
            }
            break;
        }
    }

    return messageTick;
}

void MessageProcessor::setExtension(IMessageProcessorExtension* inExtension)
{
    extension = inExtension;
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

Uint32 MessageProcessor::handleInputChangeRequest(ConnectionHandle clientHandle,
                                                  Uint8* messageBuffer,
                                                  std::size_t messageSize)
{
    // Deserialize the message.
    InputChangeRequest inputChangeRequest{};
    Deserialize::fromBuffer(messageBuffer, messageSize, inputChangeRequest);

    // Fill in the handle of the client that sent the message.
    inputChangeRequest.clientHandle = clientHandle;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<InputChangeRequest>(inputChangeRequest);

    // Return the tick number associated with this message.
    return inputChangeRequest.tickNum;
}

} // End namespace WorldServer
} // End namespace AM
