#include "ServiceMessageProcessor.h"
#include "Config.h"
#include "Deserialize.h"
#include "ChatServiceHeartbeat.h"
#include "ChatServiceHeartbeatResponse.h"
#include "Log.h"
#include "asio/error.hpp"
#include <utility>

namespace AM
{
namespace ChatServer
{

ServiceMessageProcessor::ServiceMessageProcessor(
    SendCallback inSendCallback, DisconnectCallback inDisconnectCallback)
: sendCallback{std::move(inSendCallback)}
, disconnectCallback{std::move(inDisconnectCallback)}
, messageFramer{Config::SERVICE_MAX_WRITE_PAYLOAD_SIZE}
{
}

void ServiceMessageProcessor::processReceivedMessage(
    ConnectionHandle handle, ChatServiceMessageType messageType,
    std::span<const Uint8> messageBuffer)
{
    // Match the enum values to their message types.
    switch (messageType) {
        case ChatServiceMessageType::ChatServiceHeartbeat: {
            handleMessage<ChatServiceHeartbeat>(handle, messageBuffer);
            break;
        }
        default: {
            LOG_INFO("Received unexpected service message type: %u",
                     static_cast<unsigned int>(messageType));
            disconnectCallback(handle, asio::error::make_error_code(
                                           asio::error::invalid_argument));
            break;
        }
    }
}

void ServiceMessageProcessor::handleMessage(ConnectionHandle handle,
                                            const ChatServiceHeartbeat&)
{
    // Respond so the service knows we're still here.
    sendCallback(handle,
                 messageFramer.frameMessage(ChatServiceHeartbeatResponse{}));
}

template<typename Message>
void ServiceMessageProcessor::handleMessage(
    ConnectionHandle handle, std::span<const Uint8> messageBuffer)
{
    // Deserialize the message.
    Message message{};
    if (!Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                                 message)) {
        disconnectCallback(handle, asio::error::make_error_code(
                                       asio::error::invalid_argument));
        return;
    }

    // Pass it to the appropriate handler.
    handleMessage(handle, message);
}

} // End namespace ChatServer
} // End namespace AM
