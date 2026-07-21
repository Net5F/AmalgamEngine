#include "MessageProcessor.h"
#include "Deserialize.h"
#include "Serialize.h"
#include "ByteTools.h"
#include "AccountMessageType.h"
#include "AccountRegisterRequest.h"
#include "AccountRegisterResponse.h"
#include "Log.h"
#include <span>

namespace AM
{
namespace AccountServer
{

MessageProcessor::MessageProcessor(asio::io_context& inNetworkIoContext,
                                   asio::thread_pool& inDatabasePool,
                                   SendCallback inSendCallback)
: networkIoContext{inNetworkIoContext}
, databasePool{inDatabasePool}
, sendCallback{inSendCallback}
{
}

void MessageProcessor::processReceivedMessage(
    NetworkID netID, Uint8 messageType, std::span<const Uint8> messageBuffer)
{
    // Match the enum values to their message types.
    AccountMessageType accountMessageType{
        static_cast<AccountMessageType>(messageType)};
    switch (accountMessageType) {
        case AccountMessageType::AccountRegisterRequest: {
            dispatchMessage<AccountRegisterRequest>(netID, messageBuffer);
            break;
        }
        default: {
            LOG_FATAL("Received unexpected message type: %u", messageType);
            break;
        }
    }
}

// TODO: Commit, then figure out recovery key stuff.
//       Need to map out operations: what occurs for each one? Which require 
//       database state so they can be returned to?
void MessageProcessor::handleMessage(NetworkID netID,
                                     const AccountRegisterRequest& message)
{
    AccountRegisterResponse response{};
    response.result = AccountRegisterResponse::Success;
    response.recoveryKey = "...";

    sendCallback(netID, serializeMessage(response));
}

template<typename Message>
void MessageProcessor::dispatchMessage(NetworkID netID,
                                       std::span<const Uint8> messageBuffer)
{
    // Deserialize the message.
    Message message{};
    Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                            message);

    // Pass it to the appropriate handler.
    handleMessage(netID, message);
}

template<typename Message>
BinaryBufferSharedPtr MessageProcessor::serializeMessage(const Message& message)
{
    // Allocate the buffer.
    std::size_t totalMessageSize{MESSAGE_HEADER_SIZE
                                 + Serialize::measureSize(message)};
    BinaryBufferSharedPtr messageBuffer{
        std::make_shared<BinaryBuffer>(totalMessageSize)};

    // Serialize the message struct into the buffer, leaving room for the
    // header.
    std::size_t messageSize{Serialize::toBuffer(messageBuffer->data(),
                                                messageBuffer->size(), message,
                                                MESSAGE_HEADER_SIZE)};

    // Copy the type into the buffer.
    // TODO: Add a nice compile-time message if T doesn't have MESSAGE_TYPE.
    messageBuffer->at(MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(Message::MESSAGE_TYPE);

    // Copy the messageSize into the buffer.
    ByteTools::write16(static_cast<Uint16>(messageSize),
                       messageBuffer->data() + MessageHeaderIndex::Size);

    return messageBuffer;
}

} // End namespace AccountServer
} // End namespace AM
