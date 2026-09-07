#include "AccountMessageProcessor.h"
#include "MessageProcessorContext.h"
#include "LoginResponse.h"
#include "LogoutResponse.h"
#include "RegisterResponse.h"
#include "ServiceTicketIssued.h"
#include "Deserialize.h"
#include "QueuedEvents.h"
#include "Log.h"
#include "asio/error.hpp"

namespace AM
{
namespace Client
{

AccountMessageProcessor::AccountMessageProcessor(
    const MessageProcessorContext& inMessageProcessorContext,
    DisconnectCallback inDisconnectCallback)
: networkEventDispatcher{inMessageProcessorContext.networkEventDispatcher}
, disconnectCallback{std::move(inDisconnectCallback)}
{
}

void AccountMessageProcessor::processReceivedMessage(
    AccountClientMessageType messageType, std::span<const Uint8> messageBuffer)
{
    switch (messageType) {
        case AccountClientMessageType::RegisterResponse: {
            dispatchMessage<RegisterResponse>(messageBuffer);
            break;
        }
        case AccountClientMessageType::LoginResponse: {
            dispatchMessage<LoginResponse>(messageBuffer);
            break;
        }
        case AccountClientMessageType::LogoutResponse: {
            dispatchMessage<LogoutResponse>(messageBuffer);
            break;
        }
        case AccountClientMessageType::ServiceTicketIssued: {
            dispatchMessage<ServiceTicketIssued>(messageBuffer);
            break;
        }
        default: {
            LOG_INFO("Received unexpected AccountServer message type: %u",
                     static_cast<unsigned int>(messageType));
            if (disconnectCallback) {
                disconnectCallback(asio::error::make_error_code(
                    asio::error::invalid_argument));
            }
            break;
        }
    }
}

template<typename Message>
void AccountMessageProcessor::dispatchMessage(
    std::span<const Uint8> messageBuffer)
{
    if (messageBuffer.empty()) {
        LOG_INFO("Received an empty AccountServer message payload.");
        if (disconnectCallback) {
            disconnectCallback(
                asio::error::make_error_code(asio::error::invalid_argument));
        }
        return;
    }

    Message message{};
    if (!Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                                 message)) {
        if (disconnectCallback) {
            disconnectCallback(
                asio::error::make_error_code(asio::error::invalid_argument));
        }
        return;
    }

    networkEventDispatcher.push<Message>(message);
}

} // namespace Client
} // namespace AM
