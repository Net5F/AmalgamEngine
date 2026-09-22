#include "AccountClientEndpoint.h"
#include "AccountConnectionEvent.h"
#include "MessageProcessorContext.h"
#include "UserConfig.h"
#include "Log.h"
#include "QueuedEvents.h"
#include "asio/error.hpp"
#include "asio/ip/address.hpp"
#include "asio/system_error.hpp"
#include <chrono>
#include <limits>
#include <utility>

namespace AM
{
namespace Client
{

AccountClientEndpoint::AccountClientEndpoint(
    asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
    const MessageProcessorContext& inMessageProcessorContext)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, networkEventDispatcher{inMessageProcessorContext.networkEventDispatcher}
, connectionTimer{inIoContext}
, connectingSocket{}
, connection{}
, messageFramer{MAX_WRITE_PAYLOAD_SIZE}
, pendingMessages{}
, pendingMessageBytes{0}
, messageProcessor{inMessageProcessorContext,
                   std::bind_front(&AccountClientEndpoint::disconnectWithError,
                                   this)}
, connectionState{ConnectionState::Disconnected}
, connectionAttempt{0}
{
}

AccountClientEndpoint::~AccountClientEndpoint()
{
    cancelConnectionTimer();
    asio::error_code ignoredError{};
    if (connectingSocket) {
        connectingSocket->cancel(ignoredError);
        connectingSocket->close(ignoredError);
    }
    if (connection) {
        connection->close();
    }
}

void AccountClientEndpoint::connectOnIOThread()
{
    if (connectionState != ConnectionState::Disconnected) {
        return;
    }

    connectionState = ConnectionState::Connecting;
    ServerAddress serverAddress{UserConfig::get().getAccountServerAddress()};
    beginConnect(std::move(serverAddress.IP), serverAddress.port);
}

void AccountClientEndpoint::beginConnect(std::string serverIP,
                                         unsigned int serverPort)
{
    if (connectionState != ConnectionState::Connecting) {
        return;
    }

    const std::size_t currentAttempt{++connectionAttempt};
    asio::error_code error{};
    asio::ip::address serverAddress{asio::ip::make_address(serverIP, error)};
    if (error || (serverPort > std::numeric_limits<Uint16>::max())) {
        if (!error) {
            error = asio::error::make_error_code(asio::error::invalid_argument);
        }
        handleConnectionFailure(currentAttempt, error);
        return;
    }

    connectingSocket = std::make_shared<asio::ip::tcp::socket>(ioContext);
    std::shared_ptr<asio::ip::tcp::socket> socket{connectingSocket};

    // Attempt to connect, with a timeout.
    try {
        connectionTimer.expires_after(
            std::chrono::duration_cast<asio::steady_timer::duration>(
                std::chrono::duration<double>(CONNECTING_TIMEOUT_S)));
        connectionTimer.async_wait(
            [this, socket, currentAttempt](const asio::error_code& timerError) {
                if (timerError || (currentAttempt != connectionAttempt)
                    || (connectionState != ConnectionState::Connecting)) {
                    return;
                }

                asio::error_code ignoredError{};
                socket->cancel(ignoredError);
                socket->close(ignoredError);
                handleConnectionFailure(
                    currentAttempt,
                    asio::error::make_error_code(asio::error::timed_out));
            });

        socket->async_connect(
            asio::ip::tcp::endpoint{serverAddress,
                                    static_cast<unsigned short>(serverPort)},
            [this, socket,
             currentAttempt](const asio::error_code& connectError) {
                onSocketConnected(socket, currentAttempt, connectError);
            });
    } catch (const asio::system_error& e) {
        handleConnectionFailure(currentAttempt, e.code());
    }
}

void AccountClientEndpoint::handleConnectionFailure(
    std::size_t currentAttempt, const asio::error_code& error)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    // Tear down the connection.
    connectionState = ConnectionState::Disconnected;
    ++connectionAttempt;
    cancelConnectionTimer();
    asio::error_code ignoredError{};
    if (connectingSocket) {
        connectingSocket->cancel(ignoredError);
        connectingSocket->close(ignoredError);
    }
    connectingSocket.reset();
    connection.reset();
    clearPendingMessages();

    LOG_INFO("AccountServer connection failed: %s", error.message().c_str());
    emitConnectionEvent(AccountConnectionEvent::Type::ConnectionFailed);
}

void AccountClientEndpoint::disconnectWithError(const asio::error_code& error)
{
    if (connection) {
        connection->disconnect(error);
    }
}

void AccountClientEndpoint::cancelConnectionTimer()
{
    try {
        connectionTimer.cancel();
    } catch (const asio::system_error& e) {
        LOG_INFO("Failed to cancel AccountServer connection timer: %s",
                 e.code().message().c_str());
    }
}

void AccountClientEndpoint::sendPendingMessages()
{
    while (!pendingMessages.empty()) {
        BinaryBufferSharedPtr message{pendingMessages.front()};
        if (!connection || !connection->sendFramed(std::move(message))) {
            LOG_INFO("Failed to queue an AccountServer message.");
            return;
        }

        pendingMessageBytes -= pendingMessages.front()->size();
        pendingMessages.pop_front();
    }
}

void AccountClientEndpoint::clearPendingMessages()
{
    pendingMessages.clear();
    pendingMessageBytes = 0;
}

void AccountClientEndpoint::onSocketConnected(
    std::shared_ptr<asio::ip::tcp::socket> socket, std::size_t currentAttempt,
    const asio::error_code& error)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    cancelConnectionTimer();
    if (error) {
        handleConnectionFailure(currentAttempt, error);
        return;
    }

    // Socket successfully connected. Configure and create the new connection.
    TlsTransport::ClassConfig tlsTransportConfig{
        .handshakeTimeoutS{CONNECTING_TIMEOUT_S},
        .maxQueuedWriteBytes{MAX_QUEUED_WRITE_BYTES},
        .maxQueuedMessages{MAX_QUEUED_WRITES}};
    Connection::ClassConfig connectionConfig{
        .tlsTransportConfig{tlsTransportConfig},
        .partialReceiveTimeoutS{PARTIAL_RECEIVE_TIMEOUT_S},
        .idleTimeoutS{IDLE_TIMEOUT_S},
        .maxReadPayloadSize{MAX_READ_PAYLOAD_SIZE},
        .maxWritePayloadSize{MAX_WRITE_PAYLOAD_SIZE}};

    connection = std::make_shared<Connection>(std::move(*socket), sslContext,
                                              connectionConfig);
    connectingSocket.reset();

    connection->setMessageCallback(
        [this](AccountClientMessageType messageType,
               std::span<const Uint8> messageBuffer) {
            onMessageReceived(messageType, messageBuffer);
        });
    connection->startClient(
        [this, currentAttempt](AM::Connection&) {
            onConnectionReady(currentAttempt);
        },
        [this, currentAttempt](const asio::error_code& disconnectError) {
            onConnectionDisconnected(currentAttempt, disconnectError);
        });
}

void AccountClientEndpoint::onConnectionReady(std::size_t currentAttempt)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    connectionState = ConnectionState::Connected;
    emitConnectionEvent(AccountConnectionEvent::Type::Connected);
    sendPendingMessages();
}

void AccountClientEndpoint::onConnectionDisconnected(
    std::size_t currentAttempt, const asio::error_code& error)
{
    if (currentAttempt != connectionAttempt) {
        return;
    }

    ConnectionState previousState{
        connectionState.exchange(ConnectionState::Disconnected)};
    ++connectionAttempt;
    connection.reset();
    connectingSocket.reset();
    clearPendingMessages();

    LOG_INFO("AccountServer disconnected: %s", error.message().c_str());
    if (previousState == ConnectionState::Connecting) {
        emitConnectionEvent(AccountConnectionEvent::Type::ConnectionFailed);
    }
    else if (previousState == ConnectionState::Connected) {
        emitConnectionEvent(AccountConnectionEvent::Type::Disconnected);
    }
}

void AccountClientEndpoint::onMessageReceived(
    AccountClientMessageType messageType, std::span<const Uint8> messageBuffer)
{
    messageProcessor.processReceivedMessage(messageType, messageBuffer);
}

void AccountClientEndpoint::emitConnectionEvent(
    AccountConnectionEvent::Type type)
{
    networkEventDispatcher.emplace<AccountConnectionEvent>(type);
}

} // namespace Client
} // namespace AM
