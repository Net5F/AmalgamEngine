#include "MessageProcessor.h"
#include "ServerNetworkDefs.h"
#include "Network.h"
#include "Deserialize.h"
#include "Heartbeat.h"
#include "InputChangeRequest.h"
#include "ChunkUpdateRequest.h"
#include "TileUpdateRequest.h"
#include "Log.h"

namespace AM
{
namespace Server
{
MessageProcessor::MessageProcessor(EventDispatcher& inDispatcher)
: dispatcher{inDispatcher}
{
}

Sint64 MessageProcessor::processReceivedMessage(NetworkID netID, MessageType messageType,
                                Uint8* messageBuffer,
                                unsigned int messageSize)
{
    // The tick that the received message corresponds to.
    // Will be -1 if the message doesn't correspond to any tick.
    Sint64 messageTick{-1};

    /* Match the enum values to their event types. */
    switch (messageType) {
        case MessageType::Heartbeat: {
            messageTick =
                static_cast<Sint64>(handleHeartbeat(messageBuffer, messageSize));
            break;
        }
        case MessageType::InputChangeRequest: {
            messageTick = static_cast<Sint64>(handleInputChangeRequest(netID, messageBuffer,
                messageSize));
            break;
        }
        case MessageType::ChunkUpdateRequest: {
            handleChunkUpdateRequest(netID, messageBuffer, messageSize);
            break;
        }
        case MessageType::TileUpdateRequest: {
            pushEvent<TileUpdateRequest>(messageBuffer, messageSize);
            break;
        }
        default: {
            LOG_ERROR("Received unexpected message type: %u", messageType);
        }
    }

    return messageTick;
}

template<typename T>
void MessageProcessor::pushEvent(Uint8* messageBuffer, unsigned int messageSize)
{
    // Deserialize the message.
    T message{};
    Deserialize::fromBuffer(messageBuffer, messageSize, message);

    // Push the message into any subscribed queues.
    dispatcher.push<T>(message);
}

template<typename T>
void MessageProcessor::pushEventSharedPtr(Uint8* messageBuffer, unsigned int messageSize)
{
    // Deserialize the message.
    std::shared_ptr<T> message{std::make_shared<T>()};
    Deserialize::fromBuffer(messageBuffer, messageSize, *message);

    // Push the message into any subscribed queues.
    dispatcher.push<std::shared_ptr<const T>>(message);
}

Uint32 MessageProcessor::handleHeartbeat(Uint8* messageBuffer, unsigned int messageSize)
{
    // Deserialize the message.
    Heartbeat heartbeat{};
    Deserialize::fromBuffer(messageBuffer, messageSize, heartbeat);

    // Return the tick number associated with this message.
    return heartbeat.tickNum;
}

Uint32 MessageProcessor::handleInputChangeRequest(NetworkID netID, Uint8* messageBuffer, unsigned int messageSize)
{
    // Deserialize the message.
    InputChangeRequest inputChangeRequest{};
    Deserialize::fromBuffer(messageBuffer, messageSize,
                            inputChangeRequest);

    // Fill in the network ID that we assigned to this client.
    inputChangeRequest.netID = netID;

    // Push the message into any subscribed queues.
    dispatcher.push<InputChangeRequest>(inputChangeRequest);

    // Return the tick number associated with this message.
    return inputChangeRequest.tickNum;
}

void MessageProcessor::handleChunkUpdateRequest(NetworkID netID, Uint8* messageBuffer, unsigned int messageSize)
{
    // Deserialize the message.
    ChunkUpdateRequest chunkUpdateRequest{};
    Deserialize::fromBuffer(messageBuffer, messageSize,
                            chunkUpdateRequest);

    // Fill in the network ID that we assigned to this client.
    chunkUpdateRequest.netID = netID;

    // Push the message into any subscribed queues.
    dispatcher.push<ChunkUpdateRequest>(chunkUpdateRequest);
}

} // End namespace Server
} // End namespace AM
