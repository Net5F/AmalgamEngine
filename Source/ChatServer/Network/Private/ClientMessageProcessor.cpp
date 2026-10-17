#include "ClientMessageProcessor.h"
#include "PlayerLocator.h"
#include "Config.h"
#include "SharedConfig.h"
#include "Cylinder.h"
#include "Deserialize.h"
#include "ChatConnectionRequest.h"
#include "ChatConnectionResponse.h"
#include "ChatSendRequest.h"
#include "ChatMessage.h"
#include "ChatError.h"
#include "ChatHeartbeat.h"
#include "ChatHeartbeatResponse.h"
#include "ChatHelpers.h"
#include "Shared/AccountServiceEndpoint.h"
#include "Log.h"
#include "asio/error.hpp"
#include <algorithm>
#include <utility>
#include <vector>

namespace AM
{
namespace ChatServer
{

ClientMessageProcessor::ClientMessageProcessor(
    AccountServiceEndpoint& inAccountEndpoint, PlayerLocator& inPlayerLocator,
    SendCallback inSendCallback, DisconnectCallback inDisconnectCallback)
: accountEndpoint{inAccountEndpoint}
, playerLocator{inPlayerLocator}
, sendCallback{std::move(inSendCallback)}
, disconnectCallback{std::move(inDisconnectCallback)}
, messageFramer{Config::MAX_WRITE_PAYLOAD_SIZE}
, sessions{}
, accountClients{}
{
}

void ClientMessageProcessor::processReceivedMessage(
    ConnectionHandle handle, ChatClientMessageType messageType,
    std::span<const Uint8> messageBuffer)
{
    // Get the client's session, creating it if this is their first message.
    ClientSession& session{sessions[handle]};

    // Make sure the message is allowed in the client's current state.
    // Note: A correctly-behaving client will never send a message out of
    //       order, so we treat it as a malformed message.
    bool isTicketMessage{messageType
                         == ChatClientMessageType::ChatConnectionRequest};
    bool messageAllowed{
        isTicketMessage
            ? (session.state == ClientSession::State::AwaitingTicket)
            : (session.state == ClientSession::State::Authenticated)};
    if (!messageAllowed) {
        LOG_INFO("Received client message in an invalid state. Type: %u",
                 static_cast<unsigned int>(messageType));
        disconnectCallback(handle, asio::error::make_error_code(
                                       asio::error::invalid_argument));
        return;
    }

    // Match the enum values to their message types.
    switch (messageType) {
        case ChatClientMessageType::ChatConnectionRequest: {
            handleMessage<ChatConnectionRequest>(handle, session,
                                                 messageBuffer);
            break;
        }
        case ChatClientMessageType::ChatSendRequest: {
            handleMessage<ChatSendRequest>(handle, session, messageBuffer);
            break;
        }
        case ChatClientMessageType::ChatHeartbeat: {
            handleMessage<ChatHeartbeat>(handle, session, messageBuffer);
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

void ClientMessageProcessor::onClientDisconnected(ConnectionHandle handle)
{
    auto sessionIt{sessions.find(handle)};
    if (sessionIt == sessions.end()) {
        return;
    }

    // If this client was authenticated, release its account.
    const ClientSession& session{sessionIt->second};
    if (session.state == ClientSession::State::Authenticated) {
        auto accountIt{accountClients.find(session.accountID)};
        if ((accountIt != accountClients.end())
            && (accountIt->second == handle)) {
            accountClients.erase(accountIt);
        }
    }

    sessions.erase(sessionIt);
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           ClientSession& session,
                                           const ChatConnectionRequest& message)
{
    // Ask the AccountServer to validate the ticket.
    // Note: The callback is called on the network IO thread (this thread).
    //       The client may have disconnected by the time it's called.
    session.state = ClientSession::State::ValidatingTicket;
    accountEndpoint.consumeServiceTicket(
        message.ticket,
        [this, handle](const ConsumeServiceTicketResponse& response) {
            handleTicketResponse(handle, response);
        });
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           ClientSession& session,
                                           const ChatSendRequest& message)
{
    // Validate the channel.
    // Note: A correctly-behaving client will never send an invalid channel,
    //       so we treat it as a malformed message.
    if (!isValidChatChannel(message.channel)) {
        LOG_INFO("Received chat message with invalid channel: %u",
                 static_cast<unsigned int>(message.channel));
        disconnectCallback(handle, asio::error::make_error_code(
                                       asio::error::invalid_argument));
        return;
    }

    // If the client is sending too quickly, reject the message.
    // Note: We check this first, so all requests count against the limit.
    if (!tryUseMessage(session)) {
        sendCallback(handle, messageFramer.frameMessage(
                                 ChatError{.type{ChatError::RateLimited}}));
        return;
    }

    // Validate the text.
    if (!ChatHelpers::validateChatText(message.text).success()) {
        sendCallback(handle, messageFramer.frameMessage(
                                 ChatError{.type{ChatError::InvalidText}}));
        return;
    }

    // Send the message to its recipients.
    switch (message.channel) {
        case ChatChannel::Global: {
            broadcast(messageFramer.frameMessage(
                ChatMessage{.channel{ChatChannel::Global},
                            .senderName{session.username},
                            .text{message.text}}));
            break;
        }
        case ChatChannel::Local: {
            // Note: If the sender isn't in the world (e.g. they're still
            //       loading in, or the world server is down), they have no
            //       position to send from.
            if (!sendLocal(session.accountID,
                           messageFramer.frameMessage(
                               ChatMessage{.channel{ChatChannel::Local},
                                           .senderName{session.username},
                                           .text{message.text}}))) {
                sendCallback(handle, messageFramer.frameMessage(ChatError{
                                         .type{ChatError::NotInWorld}}));
            }
            break;
        }
        default: {
            break;
        }
    }
}

void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           ClientSession&, const ChatHeartbeat&)
{
    // Respond so the client knows we're still here.
    sendCallback(handle, messageFramer.frameMessage(ChatHeartbeatResponse{}));
}

void ClientMessageProcessor::handleTicketResponse(
    ConnectionHandle handle, const ConsumeServiceTicketResponse& response)
{
    // If the client disconnected while we were waiting, there's nothing to do.
    auto sessionIt{sessions.find(handle)};
    if ((sessionIt == sessions.end())
        || (sessionIt->second.state
            != ClientSession::State::ValidatingTicket)) {
        return;
    }

    // If the ticket was rejected, tell the client.
    // Note: The client is expected to disconnect. If it doesn't, it'll be
    //       disconnected when it times out or sends another message.
    if (response.result != ConsumeServiceTicketResponse::Success) {
        sessionIt->second.state = ClientSession::State::Rejected;
        ChatConnectionResponse::Result result{
            (response.result == ConsumeServiceTicketResponse::InvalidTicket)
                ? ChatConnectionResponse::InvalidTicket
                : ChatConnectionResponse::InternalError};
        LOG_INFO("Rejected chat client: %s",
                 (result == ChatConnectionResponse::InvalidTicket)
                     ? "Invalid ticket."
                     : "Internal error.");
        sendCallback(handle, messageFramer.frameMessage(
                                 ChatConnectionResponse{.result{result}}));
        return;
    }

    // If this account is already logged in, disconnect the old client.
    auto accountIt{accountClients.find(response.accountID)};
    if (accountIt != accountClients.end()) {
        LOG_INFO("Disconnecting chat client: Account logged in from another "
                 "connection. AccountID: %lld",
                 static_cast<long long>(response.accountID));
        ConnectionHandle oldHandle{accountIt->second};
        accountClients.erase(accountIt);
        disconnectCallback(
            oldHandle, asio::error::make_error_code(asio::error::shut_down));
    }

    // Authenticate the client.
    // Note: We re-find the session, in case disconnecting the old client
    //       modified the map.
    ClientSession& session{sessions.at(handle)};
    session.state = ClientSession::State::Authenticated;
    session.accountID = response.accountID;
    session.username = response.username;
    session.availableMessages = Config::CHAT_MESSAGE_BURST_LIMIT;
    session.lastRefillTime = std::chrono::steady_clock::now();
    accountClients[response.accountID] = handle;

    LOG_INFO("Chat client authenticated. AccountID: %lld",
             static_cast<long long>(response.accountID));
    sendCallback(handle, messageFramer.frameMessage(ChatConnectionResponse{
                             .result{ChatConnectionResponse::Success}}));
}

bool ClientMessageProcessor::tryUseMessage(ClientSession& session)
{
    // Refill based on how long it's been since the last refill.
    std::chrono::steady_clock::time_point currentTime{
        std::chrono::steady_clock::now()};
    double elapsedS{
        std::chrono::duration<double>(currentTime - session.lastRefillTime)
            .count()};
    session.availableMessages
        = std::min(Config::CHAT_MESSAGE_BURST_LIMIT,
                   session.availableMessages
                       + (elapsedS / Config::CHAT_MESSAGE_REFILL_INTERVAL_S));
    session.lastRefillTime = currentTime;

    // Try to use a message.
    if (session.availableMessages < 1) {
        return false;
    }
    session.availableMessages -= 1;
    return true;
}

void ClientMessageProcessor::broadcast(BinaryBufferSharedPtr message)
{
    // Gather the recipients first.
    // Note: A failed send will disconnect the client, which removes it from
    //       our session map, so we can't send while iterating it.
    std::vector<ConnectionHandle> recipients{};
    recipients.reserve(sessions.size());
    for (const auto& [handle, session] : sessions) {
        if (session.state == ClientSession::State::Authenticated) {
            recipients.push_back(handle);
        }
    }

    // Send the message.
    // Note: All recipients share the same framed buffer.
    for (ConnectionHandle handle : recipients) {
        sendCallback(handle, message);
    }
}

bool ClientMessageProcessor::sendLocal(Sint64 senderAccountID,
                                       BinaryBufferSharedPtr message)
{
    const Position* senderPosition{playerLocator.getPosition(senderAccountID)};
    if (!senderPosition) {
        return false;
    }

    // Gather the recipients first.
    // Note: The sender is within range of themselves, so they'll be included.
    // Note: A failed send will disconnect the client, which removes it from
    //       accountClients, so we can't send while iterating.
    std::vector<ConnectionHandle> recipients{};
    for (Sint64 accountID : playerLocator.getPlayers(
             Cylinder{*senderPosition, SharedConfig::LOCAL_CHAT_RADIUS,
                      SharedConfig::LOCAL_CHAT_HALF_HEIGHT})) {
        // Note: A player may be in the world without being connected to chat.
        auto accountIt{accountClients.find(accountID)};
        if (accountIt != accountClients.end()) {
            recipients.push_back(accountIt->second);
        }
    }

    // Send the message.
    // Note: All recipients share the same framed buffer.
    for (ConnectionHandle handle : recipients) {
        sendCallback(handle, message);
    }

    return true;
}

template<typename Message>
void ClientMessageProcessor::handleMessage(ConnectionHandle handle,
                                           ClientSession& session,
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
    handleMessage(handle, session, message);
}

} // End namespace ChatServer
} // End namespace AM
