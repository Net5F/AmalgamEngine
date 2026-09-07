#pragma once

#include "AccountClientMessageType.h"
#include "AccountConnectionEvent.h"
#include "AccountMessageProcessor.h"
#include "SimpleConnection.h"
#include "Log.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/post.hpp"
#include "asio/ssl/context.hpp"
#include "asio/steady_timer.hpp"
#include <atomic>
#include <cstddef>
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
 * Manages communication with the AccountServer.
 */
class AccountClientEndpoint
{
public:
    enum class ConnectionState { Disconnected, Connecting, Connected };

    AccountClientEndpoint(
        asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
        const MessageProcessorContext& inMessageProcessorContext);

    ~AccountClientEndpoint();

    /**
     * Starts an asynchronous connection to the configured AccountServer.
     */
    void connect();

    /**
     * Disconnects from the AccountServer, or cancels an active attempt.
     */
    void disconnect();

    ConnectionState getConnectionState() const;

    /**
     * Queues a typed message to be sent on the network thread.
     */
    template<typename Message>
    void send(const Message& message);

private:
    using Connection = SimpleConnection<AccountClientMessageType>;

    // Note: If a project ever cares to configure any of these, they can be
    //       moved into Config.h.
    static constexpr double CONNECTING_TIMEOUT_S{5};
    static constexpr double PARTIAL_RECEIVE_TIMEOUT_S{10};
    static constexpr double IDLE_TIMEOUT_S{10};
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{4000};
    static constexpr std::size_t MAX_QUEUED_WRITES{10};
    static constexpr Uint16 MAX_READ_PAYLOAD_SIZE{500};
    static constexpr Uint16 MAX_WRITE_PAYLOAD_SIZE{500};

    void beginConnect(std::string serverIP, unsigned int serverPort);
    void onSocketConnected(std::shared_ptr<asio::ip::tcp::socket> socket,
                           std::size_t connectionAttempt,
                           const asio::error_code& error);
    void onConnectionReady(std::size_t connectionAttempt);
    void onConnectionDisconnected(std::size_t connectionAttempt,
                                  const asio::error_code& error);
    void onMessageReceived(AccountClientMessageType messageType,
                           std::span<const Uint8> messageBuffer);
    void disconnectWithError(const asio::error_code& error);
    void handleConnectionFailure(std::size_t connectionAttempt,
                                 const asio::error_code& error);
    void cancelConnectionTimer();
    void disconnectOnIOThread();
    void emitConnectionEvent(AccountConnectionEvent::Type type);

    template<typename Message>
    void sendOnIOThread(Message message);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;
    EventDispatcher& networkEventDispatcher;

    asio::steady_timer connectionTimer;
    std::shared_ptr<asio::ip::tcp::socket> connectingSocket;
    std::shared_ptr<Connection> connection;

    AccountMessageProcessor messageProcessor;

    std::atomic<ConnectionState> connectionState;
    /** Used to track which connection attempt we're on, so late async
        callbacks for already-canceled attempts don't get processed. */
    std::size_t connectionAttempt;
};

template<typename Message>
void AccountClientEndpoint::send(const Message& message)
{
    using DeclaredMessageType
        = std::remove_cv_t<decltype(Message::MESSAGE_TYPE)>;
    static_assert(std::is_same_v<DeclaredMessageType, AccountClientMessageType>,
                  "Message::MESSAGE_TYPE is not an AccountClientMessageType.");

    asio::post(ioContext, [this, message]() mutable {
        sendOnIOThread(std::move(message));
    });
}

template<typename Message>
void AccountClientEndpoint::sendOnIOThread(Message message)
{
    if ((connectionState != ConnectionState::Connected) || !connection) {
        LOG_INFO("Tried to send while the AccountServer is disconnected.");
        return;
    }

    if (!connection->send(message)) {
        LOG_INFO("Failed to queue an AccountServer message.");
    }
}

} // namespace Client
} // namespace AM
