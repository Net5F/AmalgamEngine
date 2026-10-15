#pragma once

#include "ChatConnectionEvent.h"
#include "ChatDefs.h"
#include "ChatError.h"
#include "ChatMessage.h"
#include "ServiceTicketResponse.h"
#include "Timer.h"
#include "QueuedEvents.h"
#include "entt/signal/sigh.hpp"
#include <string>

namespace AM
{
namespace Client
{
class Network;
class AccountSession;

/**
 * Owns the client's ChatServer session.
 *
 * While the account is logged in, this keeps us connected to the ChatServer:
 * it requests a ChatServer ticket, connects with it, and if the connection
 * fails or is lost, waits (with backoff) and repeats with a new ticket. When
 * the account logs out (or starts logging out), the connection is closed.
 *
 * Received chat messages and errors are passed on through our signals.
 *
 * All public functions, including tick(), must be called from the main thread.
 */
class ChatSession
{
public:
    enum class State {
        /** Not logged in, so not connected. */
        Disconnected,
        /** Waiting for the AccountServer to give us a ChatServer ticket. */
        RequestingTicket,
        /** Connecting to and authenticating with the ChatServer. */
        Connecting,
        /** Authenticated. Chat can be sent and received. */
        Connected,
        /** The last attempt failed or the connection was lost. Waiting to try
            again. */
        WaitingToReconnect
    };

    ChatSession(Network& inNetwork, AccountSession& inAccountSession,
                EventDispatcher& networkEventDispatcher);
    ~ChatSession();

    ChatSession(const ChatSession&) = delete;
    ChatSession& operator=(const ChatSession&) = delete;

    /**
     * Processes events received from the ChatServer, and connects or
     * disconnects to match the account's login state.
     *
     * Must be called after AccountSession::tick().
     */
    void tick();

    /**
     * Sends the given text to the given channel.
     *
     * Note: The text isn't validated here. Callers should check it with
     *       ChatHelpers::validateChatText(), so they can show the user why
     *       it was invalid. The ChatServer will reply with a ChatError if
     *       it's invalid.
     *
     * @return false if we aren't connected, else true.
     */
    bool sendMessage(ChatChannel channel, const std::string& text);

    State getState() const noexcept;

    /** Returns true if we're connected to the ChatServer. */
    bool isConnected() const noexcept;

private:
    /** The delay before our first reconnect attempt. Doubles with each
        consecutive failure, up to MAX_RECONNECT_DELAY_S. */
    static constexpr double MIN_RECONNECT_DELAY_S{1};
    static constexpr double MAX_RECONNECT_DELAY_S{30};

    void handleConnectionEvent(const ChatConnectionEvent& event);
    void onServiceTicketReceived(const ServiceTicketResponse& response);

    /** Connects or disconnects to match the account's login state. */
    void updateConnection();

    void requestTicket();
    void scheduleReconnect();
    void disconnect();
    void setState(State newState);

    Network& network;
    AccountSession& accountSession;

    EventQueue<ChatConnectionEvent> connectionEventQueue;
    EventQueue<ChatMessage> chatMessageQueue;
    EventQueue<ChatError> chatErrorQueue;

    State state;

    /** The ID of our current ChatClientEndpoint connection. Events for other
        connections are stale and get discarded. */
    Uint32 connectionID;

    /** Tracks how long we've been waiting to reconnect. */
    Timer reconnectTimer;
    /** How long to wait before the next reconnect attempt. */
    double reconnectDelayS;

    entt::sigh<void(State)> stateChangedSig;
    entt::sigh<void(const ChatMessage&)> messageReceivedSig;
    entt::sigh<void(ChatError::Type)> errorReceivedSig;

public:
    /** Our state changed. Use this to show the chat connection status. */
    entt::sink<entt::sigh<void(State)>> stateChanged;

    /** A chat message was received (including our own sent messages). */
    entt::sink<entt::sigh<void(const ChatMessage&)>> messageReceived;

    /** The ChatServer rejected one of our sent messages. */
    entt::sink<entt::sigh<void(ChatError::Type)>> errorReceived;
};

} // namespace Client
} // namespace AM
