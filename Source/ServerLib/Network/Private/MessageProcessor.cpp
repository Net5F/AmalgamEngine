#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "DispatchMessage.h"
#include "IMessageProcessorExtension.h"
#include "ServerNetworkDefs.h"
#include "Heartbeat.h"
#include "InputChangeRequest.h"
#include "ChunkUpdateRequest.h"
#include "TileUpdateRequest.h"
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
                                                MessageType messageType,
                                                Uint8* messageBuffer,
                                                unsigned int messageSize)
{
    // The tick that the received message corresponds to.
    // Will be -1 if the message doesn't correspond to any tick.
    Sint64 messageTick{-1};

    // Match the enum values to their event types.
    switch (messageType) {
        case MessageType::Heartbeat: {
            messageTick = static_cast<Sint64>(
                handleHeartbeat(messageBuffer, messageSize));
            break;
        }
        case MessageType::InputChangeRequest: {
            messageTick = static_cast<Sint64>(
                handleInputChangeRequest(netID, messageBuffer, messageSize));
            break;
        }
        case MessageType::ChunkUpdateRequest: {
            handleChunkUpdateRequest(netID, messageBuffer, messageSize);
            break;
        }
        case MessageType::TileUpdateRequest: {
            dispatchMessage<TileUpdateRequest>(messageBuffer, messageSize,
                                               networkEventDispatcher);
            break;
        }
        default: {
            LOG_FATAL("Received unexpected message type: %u", messageType);
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
                                         unsigned int messageSize)
{
    // Deserialize the message.
    Heartbeat heartbeat{};
    Deserialize::fromBuffer(messageBuffer, messageSize, heartbeat);

    // Return the tick number associated with this message.
    return heartbeat.tickNum;
}

Uint32 MessageProcessor::handleInputChangeRequest(NetworkID netID,
                                                  Uint8* messageBuffer,
                                                  unsigned int messageSize)
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
                                                unsigned int messageSize)
{
    // Deserialize the message.
    ChunkUpdateRequest chunkUpdateRequest{};
    Deserialize::fromBuffer(messageBuffer, messageSize, chunkUpdateRequest);

    // Fill in the network ID that we assigned to this client.
    chunkUpdateRequest.netID = netID;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<ChunkUpdateRequest>(chunkUpdateRequest);
}

} // End namespace Server
} // End namespace AM
