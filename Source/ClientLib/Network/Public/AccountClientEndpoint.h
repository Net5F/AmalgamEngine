#pragma once

#include "AccountClientMessageType.h"
#include "AccountConnectionEvent.h"
#include "AccountMessageProcessor.h"
#include "SimpleConnection.h"
#include "SimpleMessageFramer.h"
#include "Log.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/post.hpp"
#include "asio/ssl/context.hpp"
#include "asio/steady_timer.hpp"
#include <atomic>
#include <cstddef>
#include <deque>
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
 *
 * This connection behaves like a webpage talking to a web server: clients 
 * connect lazily when user input triggers a message, connection stays open as
 * long as messages are flowing, disconnect on timeout, repeat as needed.
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
     * Queues a typed message to be sent on the network thread. If necessary,
     * a connection to the AccountServer is established first.
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

    /** Connection logic */
    void connectOnIOThread();
    void beginConnect(std::string serverIP, unsigned int serverPort);
    void handleConnectionFailure(std::size_t connectionAttempt,
                                 const asio::error_code& error);
    void disconnectWithError(const asio::error_code& error);
    void cancelConnectionTimer();

    void sendPendingMessages();
    void clearPendingMessages();

    /** Event handlers. */
    void onSocketConnected(std::shared_ptr<asio::ip::tcp::socket> socket,
                           std::size_t connectionAttempt,
                           const asio::error_code& error);
    void onConnectionReady(std::size_t connectionAttempt);
    void onConnectionDisconnected(std::size_t connectionAttempt,
                                  const asio::error_code& error);
    void onMessageReceived(AccountClientMessageType messageType,
                           std::span<const Uint8> messageBuffer);

    void emitConnectionEvent(AccountConnectionEvent::Type type);

    template<typename Message>
    void sendOnIOThread(Message message);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;
    EventDispatcher& networkEventDispatcher;

    asio::steady_timer connectionTimer;
    std::shared_ptr<asio::ip::tcp::socket> connectingSocket;
    std::shared_ptr<Connection> connection;

    /** Used to queue messages while we're waiting for a connection to be
        established. */
    SimpleMessageFramer<AccountClientMessageType> messageFramer;
    std::deque<BinaryBufferSharedPtr> pendingMessages;
    std::size_t pendingMessageBytes;

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
    BinaryBufferSharedPtr framedMessage{messageFramer.frameMessage(message)};
    if (!framedMessage) {
        return;
    }

    if (connectionState == ConnectionState::Connected) {
        if (!connection || !connection->sendFramed(std::move(framedMessage))) {
            LOG_INFO("Failed to queue an AccountServer message.");
        }
        return;
    }

    const std::size_t messageSize{framedMessage->size()};
    if ((pendingMessages.size() >= MAX_QUEUED_WRITES)
        || (messageSize > MAX_QUEUED_WRITE_BYTES)
        || (pendingMessageBytes > (MAX_QUEUED_WRITE_BYTES - messageSize))) {
        LOG_INFO("Failed to queue an AccountServer message.");
        return;
    }

    pendingMessageBytes += messageSize;
    pendingMessages.push_back(std::move(framedMessage));

    if (connectionState == ConnectionState::Disconnected) {
        connectOnIOThread();
    }
}

} // namespace Client
} // namespace AM
