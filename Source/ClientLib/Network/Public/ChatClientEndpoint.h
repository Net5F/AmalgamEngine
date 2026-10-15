#pragma once

#include "ChatClientMessageType.h"
#include "ChatConnectionEvent.h"
#include "AccountDefs.h"
#include "SimpleConnection.h"
#include "Log.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/post.hpp"
#include "asio/ssl/context.hpp"
#include "asio/steady_timer.hpp"
#include <array>
#include <atomic>
#include <memory>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

namespace AM
{
class EventDispatcher;

namespace Client
{
struct MessageProcessorContext;

/**
 * Manages communication with the ChatServer.
 *
 * Unlike the AccountServer connection, this connection is held open for as
 * long as we're logged in. Once authenticated, we send periodic heartbeats
 * (which the ChatServer responds to). If we stop receiving messages, we assume
 * the ChatServer is gone and disconnect on timeout.
 *
 * Each connection consumes a single-use ticket, so this class doesn't
 * reconnect on its own. Instead, it pushes a ChatConnectionEvent when the
 * connection ends, and ChatSession reconnects with a new ticket.
 *
 * Received ChatMessage and ChatError messages are pushed into the network
 * event dispatcher.
 */
class ChatClientEndpoint
{
public:
    ChatClientEndpoint(
        asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
        const MessageProcessorContext& inMessageProcessorContext);

    ~ChatClientEndpoint();

    /**
     * Connects to the ChatServer and authenticates using the given ticket.
     * If a connection is already open, it's closed first.
     *
     * @return An ID for this connection. All ChatConnectionEvents for it will
     *         carry this ID.
     */
    Uint32 connect(const std::array<Uint8, SERVICE_TICKET_BYTES>& ticket);

    /**
     * Closes the current connection, if there is one.
     * No ChatConnectionEvent is pushed for an intentional disconnect.
     */
    void disconnect();

    /**
     * Queues a typed message to be sent on the network thread.
     * If we aren't authenticated, the message is dropped.
     */
    template<typename Message>
    void send(const Message& message);

private:
    using Connection = SimpleConnection<ChatClientMessageType>;

    // Note: If a project ever cares to configure any of these, they can be
    //       moved into Config.h.
    static constexpr double CONNECTING_TIMEOUT_S{5};
    static constexpr double PARTIAL_RECEIVE_TIMEOUT_S{10};
    /** How often we send a heartbeat while authenticated. Must be well below
        the ChatServer's idle timeout. */
    static constexpr double HEARTBEAT_INTERVAL_S{10};
    /** If we receive nothing (including heartbeat responses) for this long,
        we assume the ChatServer is gone. */
    static constexpr double IDLE_TIMEOUT_S{30};
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{4000};
    static constexpr std::size_t MAX_QUEUED_WRITES{20};
    static constexpr Uint16 MAX_READ_PAYLOAD_SIZE{500};
    static constexpr Uint16 MAX_WRITE_PAYLOAD_SIZE{500};

    enum class ConnectionState {
        Disconnected,
        /** Establishing the TCP connection and TLS handshake. */
        Connecting,
        /** Waiting for the ChatServer to validate our ticket. */
        Authenticating,
        Authenticated
    };

    /** Connection logic */
    void connectOnIOThread(Uint32 connectionID,
                           std::array<Uint8, SERVICE_TICKET_BYTES> ticket);
    void closeOnIOThread();
    void handleConnectionFailure(Uint32 connectionID,
                                 const asio::error_code& error);
    void disconnectWithError(const asio::error_code& error);
    void cancelTimer(asio::steady_timer& timer);

    void startHeartbeatTimer(Uint32 connectionID);

    /** Event handlers. */
    void onSocketConnected(std::shared_ptr<asio::ip::tcp::socket> socket,
                           Uint32 connectionID, const asio::error_code& error);
    void onConnectionReady(Uint32 connectionID);
    void onConnectionDisconnected(Uint32 connectionID,
                                  const asio::error_code& error);
    void onMessageReceived(ChatClientMessageType messageType,
                           std::span<const Uint8> messageBuffer);

    /**
     * Deserializes the given buffer into message.
     * If it fails, disconnects and returns false.
     */
    template<typename Message>
    bool deserialize(std::span<const Uint8> messageBuffer, Message& message);

    void emitConnectionEvent(Uint32 connectionID,
                             ChatConnectionEvent::Type type);

    template<typename Message>
    void sendOnIOThread(const Message& message);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;
    EventDispatcher& networkEventDispatcher;

    asio::steady_timer connectionTimer;
    asio::steady_timer heartbeatTimer;
    std::shared_ptr<asio::ip::tcp::socket> connectingSocket;
    std::shared_ptr<Connection> connection;

    /** The ticket to present once connected. Cleared after it's sent. */
    std::array<Uint8, SERVICE_TICKET_BYTES> pendingTicket;

    /** Only accessed on the IO thread. */
    ConnectionState connectionState;

    /** The ID of the connection that the IO thread is currently working on.
        Used to discard late async callbacks for old connections. */
    Uint32 currentConnectionID;

    /** Used to hand out connection IDs. Accessed on the main thread. */
    std::atomic<Uint32> nextConnectionID;
};

template<typename Message>
void ChatClientEndpoint::send(const Message& message)
{
    using DeclaredMessageType
        = std::remove_cv_t<decltype(Message::MESSAGE_TYPE)>;
    static_assert(std::is_same_v<DeclaredMessageType, ChatClientMessageType>,
                  "Message::MESSAGE_TYPE is not a ChatClientMessageType.");

    asio::post(ioContext, [this, message]() { sendOnIOThread(message); });
}

template<typename Message>
void ChatClientEndpoint::sendOnIOThread(const Message& message)
{
    if ((connectionState != ConnectionState::Authenticated) || !connection) {
        LOG_INFO("Dropped a ChatServer message: Not authenticated.");
        return;
    }

    if (!connection->send(message)) {
        LOG_INFO("Failed to queue a ChatServer message.");
    }
}

} // namespace Client
} // namespace AM
