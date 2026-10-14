#include "ClientMessageProcessor.h"
#include "Config.h"
#include "Deserialize.h"
#include "ChatHeartbeat.h"
#include "ChatHeartbeatResponse.h"
#include "Log.h"
#include "asio/error.hpp"
#include <utility>

namespace AM
{
namespace ChatServer
{

ClientMessageProcessor::ClientMessageProcessor(
    SendCallback inSendCallback, DisconnectCallback inDisconnectCallback)
: sendCallback{std::move(inSendCallback)}
, disconnectCallback{std::move(inDisconnectCallback)}
, messageFramer{Config::MAX_WRITE_PAYLOAD_SIZE}
{
}

void ClientMessageProcessor::processReceivedMessage(
    ConnectionHandle handle, ChatClientMessageType messageType,
    std::span<const Uint8> messageBuffer)
{
    // Match the enum values to their message types.
    switch (messageType) {
        case ChatClientMessageType::ChatHeartbeat: {
            handleMessage<ChatHeartbeat>(handle, messageBuffer);
            break;
        }
        default: {
            LOG_INFO("Received unexpected client message type: %u",
                     static_cast<unsigned int>(messageType));
            disconnectCallback(handle, asio::error::make_error_code(
                                           asio::error::invalid_argument));
            break;
        }
    }
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           const ChatHeartbeat&)
{
    // Respond so the client knows we're still here.
    sendCallback(handle, messageFramer.frameMessage(ChatHeartbeatResponse{}));
}

template<typename Message>
void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           std::span<const Uint8> messageBuffer)
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
