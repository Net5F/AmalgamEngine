#include "ChatSession.h"
#include "AccountSession.h"
#include "Network.h"
#include "ChatSendRequest.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
namespace Client
{

ChatSession::ChatSession(Network& inNetwork, AccountSession& inAccountSession,
                         EventDispatcher& networkEventDispatcher)
: network{inNetwork}
, accountSession{inAccountSession}
, connectionEventQueue{networkEventDispatcher}
, chatMessageQueue{networkEventDispatcher}
, chatErrorQueue{networkEventDispatcher}
, state{State::Disconnected}
, connectionID{0}
, reconnectTimer{}
, reconnectDelayS{MIN_RECONNECT_DELAY_S}
, stateChangedSig{}
, messageReceivedSig{}
, errorReceivedSig{}
, stateChanged{stateChangedSig}
, messageReceived{messageReceivedSig}
, errorReceived{errorReceivedSig}
{
    accountSession.serviceTicketRequestCompleted
        .connect<&ChatSession::onServiceTicketReceived>(*this);
}

ChatSession::~ChatSession()
{
    accountSession.serviceTicketRequestCompleted
        .disconnect<&ChatSession::onServiceTicketReceived>(*this);
}

void ChatSession::tick()
{
    ChatConnectionEvent connectionEvent{};
    while (connectionEventQueue.pop(connectionEvent)) {
        handleConnectionEvent(connectionEvent);
    }

    // Note: Messages and errors may arrive from a connection that we've since
    //       closed. We only pass them on while connected.
    ChatMessage chatMessage{};
    while (chatMessageQueue.pop(chatMessage)) {
        if (state == State::Connected) {
            messageReceivedSig.publish(chatMessage);
        }
    }

    ChatError chatError{};
    while (chatErrorQueue.pop(chatError)) {
        if (state == State::Connected) {
            errorReceivedSig.publish(chatError.type);
        }
    }

    updateConnection();
}

bool ChatSession::sendMessage(ChatChannel channel, const std::string& text)
{
    if (state != State::Connected) {
        return false;
    }

    network.chatEndpoint.send(ChatSendRequest{channel, text});
    return true;
}

ChatSession::State ChatSession::getState() const noexcept
{
    return state;
}

bool ChatSession::isConnected() const noexcept
{
    return state == State::Connected;
}

void ChatSession::handleConnectionEvent(const ChatConnectionEvent& event)
{
    // Ignore events from connections that we've moved on from.
    if ((event.connectionID != connectionID)
        || ((state != State::Connecting) && (state != State::Connected))) {
        return;
    }

    switch (event.type) {
        case ChatConnectionEvent::Type::Authenticated: {
            reconnectDelayS = MIN_RECONNECT_DELAY_S;
            setState(State::Connected);
            break;
        }
        case ChatConnectionEvent::Type::Disconnected: {
            // We were connected, so start our backoff over.
            LOG_INFO("Lost ChatServer connection.");
            reconnectDelayS = MIN_RECONNECT_DELAY_S;
            scheduleReconnect();
            break;
        }
        case ChatConnectionEvent::Type::InvalidTicket: {
            LOG_INFO("ChatServer rejected our ticket.");
            scheduleReconnect();
            break;
        }
        case ChatConnectionEvent::Type::Rejected: {
            LOG_INFO("ChatServer failed to validate our ticket.");
            scheduleReconnect();
            break;
        }
        case ChatConnectionEvent::Type::ConnectionFailed: {
            scheduleReconnect();
            break;
        }
    }
}

void ChatSession::onServiceTicketReceived(const ServiceTicketResponse& response)
{
    if ((response.audience != ServiceTicketAudience::ChatServer)
        || (state != State::RequestingTicket)) {
        return;
    }

    switch (response.result) {
        case ServiceTicketResponse::Success: {
            connectionID = network.chatEndpoint.connect(response.ticket);
            setState(State::Connecting);
            break;
        }
        case ServiceTicketResponse::InvalidSession: {
            // AccountSession has logged us out. updateConnection() will
            // handle it.
            setState(State::Disconnected);
            break;
        }
        case ServiceTicketResponse::RateLimited:
        case ServiceTicketResponse::InternalError: {
            LOG_INFO("Failed to get a ChatServer ticket.");
            scheduleReconnect();
            break;
        }
    }
}

void ChatSession::updateConnection()
{
    // Note: We disconnect as soon as a logout starts. If the logout fails,
    //       we'll reconnect.
    bool shouldBeConnected{accountSession.getLoginState()
                           == AccountSession::LoginState::LoggedIn};
    if (!shouldBeConnected) {
        if (state != State::Disconnected) {
            disconnect();
        }
        return;
    }

    if (state == State::Disconnected) {
        reconnectDelayS = MIN_RECONNECT_DELAY_S;
        requestTicket();
    }
    else if ((state == State::WaitingToReconnect)
             && (reconnectTimer.getTime() >= reconnectDelayS)) {
        reconnectDelayS = std::min(reconnectDelayS * 2, MAX_RECONNECT_DELAY_S);
        requestTicket();
    }
}

void ChatSession::requestTicket()
{
    if (accountSession.requestServiceTicket(
            ServiceTicketAudience::ChatServer)) {
        setState(State::RequestingTicket);
    }
    else {
        scheduleReconnect();
    }
}

void ChatSession::scheduleReconnect()
{
    reconnectTimer.reset();
    setState(State::WaitingToReconnect);
}

void ChatSession::disconnect()
{
    // Note: If a ticket request is in flight, its response will be ignored
    //       since we're no longer in the RequestingTicket state.
    if ((state == State::Connecting) || (state == State::Connected)) {
        network.chatEndpoint.disconnect();
    }

    setState(State::Disconnected);
}

void ChatSession::setState(State newState)
{
    if (newState == state) {
        return;
    }

    state = newState;
    stateChangedSig.publish(state);
}

} // namespace Client
} // namespace AM
