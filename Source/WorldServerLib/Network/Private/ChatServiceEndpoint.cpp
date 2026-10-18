#include "ChatServiceEndpoint.h"
#include "ChatServiceHeartbeat.h"
#include "asio/error.hpp"
#include "asio/ip/address.hpp"
#include "asio/system_error.hpp"
#include <chrono>
#include <limits>

namespace AM
{
namespace WorldServer
{

ChatServiceEndpoint::ChatServiceEndpoint(asio::io_context& inIoContext,
                                         asio::ssl::context& inSSLContext,
                                         const std::string& inServerIP,
                                         unsigned int inServerPort)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, serverIP{inServerIP}
, serverPort{inServerPort}
, connectionTimer{inIoContext}
, reconnectTimer{inIoContext}
, heartbeatTimer{inIoContext}
, connectingSocket{}
, connection{}
, reconnectDelayS{RECONNECT_MIN_DELAY_S}
, started{false}
, connectionState{ConnectionState::Disconnected}
, connectionAttempt{0}
{
}

ChatServiceEndpoint::~ChatServiceEndpoint()
{
    cancelTimer(connectionTimer);
    cancelTimer(reconnectTimer);
    cancelTimer(heartbeatTimer);
    asio::error_code ignoredError{};
    if (connectingSocket) {
        connectingSocket->cancel(ignoredError);
        connectingSocket->close(ignoredError);
    }
    if (connection) {
        connection->close();
    }
}

void ChatServiceEndpoint::start()
{
    asio::post(ioContext, [this]() {
        if (started) {
            LOG_INFO("Attempted to start ChatServiceEndpoint more than once.");
            return;
        }

        started = true;
        connectOnIOThread();
    });
}

void ChatServiceEndpoint::connectOnIOThread()
{
    if (connectionState != ConnectionState::Disconnected) {
        return;
    }

    connectionState = ConnectionState::Connecting;
    beginConnect();
}

void ChatServiceEndpoint::beginConnect()
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
        connectionTimer.expires_after(toTimerDuration(CONNECTING_TIMEOUT_S));
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

void ChatServiceEndpoint::handleConnectionFailure(std::size_t currentAttempt,
                                                  const asio::error_code& error)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    // Tear down the connection.
    connectionState = ConnectionState::Disconnected;
    ++connectionAttempt;
    cancelTimer(connectionTimer);
    asio::error_code ignoredError{};
    if (connectingSocket) {
        connectingSocket->cancel(ignoredError);
        connectingSocket->close(ignoredError);
    }
    connectingSocket.reset();
    connection.reset();

    LOG_INFO("ChatServer connection failed: %s", error.message().c_str());
    scheduleReconnect();
}

void ChatServiceEndpoint::disconnectWithError(const asio::error_code& error)
{
    // Note: We hold a local reference, since disconnecting will reset our
    //       member.
    if (std::shared_ptr<Connection> activeConnection{connection}) {
        activeConnection->disconnect(error);
    }
}

void ChatServiceEndpoint::scheduleReconnect()
{
    LOG_INFO("Reconnecting to ChatServer in %.0fs.", reconnectDelayS);

    try {
        reconnectTimer.expires_after(toTimerDuration(reconnectDelayS));
        reconnectTimer.async_wait([this](const asio::error_code& timerError) {
            if (timerError) {
                return;
            }
            connectOnIOThread();
        });
    } catch (const asio::system_error& e) {
        LOG_ERROR("Failed to schedule ChatServer reconnect: %s",
                  e.code().message().c_str());
    }

    reconnectDelayS = std::min(reconnectDelayS * 2, RECONNECT_MAX_DELAY_S);
}

void ChatServiceEndpoint::scheduleHeartbeat(std::size_t currentAttempt)
{
    try {
        heartbeatTimer.expires_after(toTimerDuration(HEARTBEAT_INTERVAL_S));
        heartbeatTimer.async_wait(
            [this, currentAttempt](const asio::error_code& timerError) {
                if (timerError || (currentAttempt != connectionAttempt)
                    || (connectionState != ConnectionState::Connected)) {
                    return;
                }

                // Note: A failed send will disconnect and reset our member.
                //       In that case, we'll reconnect, so there's nothing
                //       else to do here.
                sendOnIOThread(ChatServiceHeartbeat{});

                if ((currentAttempt == connectionAttempt)
                    && (connectionState == ConnectionState::Connected)) {
                    scheduleHeartbeat(currentAttempt);
                }
            });
    } catch (const asio::system_error& e) {
        disconnectWithError(e.code());
    }
}

void ChatServiceEndpoint::onSocketConnected(
    std::shared_ptr<asio::ip::tcp::socket> socket, std::size_t currentAttempt,
    const asio::error_code& error)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    cancelTimer(connectionTimer);
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

    // Register callbacks and start the handshake process.
    // Note: ChatServer's certificate is verified against its pin during
    //       the handshake, and it verifies ours.
    connection->setMessageCallback(
        [this](ChatServiceMessageType messageType,
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

void ChatServiceEndpoint::onConnectionReady(std::size_t currentAttempt)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    connectionState = ConnectionState::Connected;
    reconnectDelayS = RECONNECT_MIN_DELAY_S;
    LOG_INFO("Connected to ChatServer.");
    scheduleHeartbeat(currentAttempt);
}

void ChatServiceEndpoint::onConnectionDisconnected(
    std::size_t currentAttempt, const asio::error_code& error)
{
    if (currentAttempt != connectionAttempt) {
        return;
    }

    ConnectionState previousState{connectionState};
    connectionState = ConnectionState::Disconnected;
    ++connectionAttempt;
    cancelTimer(heartbeatTimer);
    connection.reset();
    connectingSocket.reset();

    if (previousState == ConnectionState::Connecting) {
        LOG_INFO("ChatServer connection failed: %s", error.message().c_str());
    }
    else {
        LOG_INFO("ChatServer disconnected: %s", error.message().c_str());
    }

    scheduleReconnect();
}

void ChatServiceEndpoint::onMessageReceived(ChatServiceMessageType messageType,
                                            std::span<const Uint8>)
{
    switch (messageType) {
        case ChatServiceMessageType::ChatServiceHeartbeatResponse: {
            // Nothing to do. Receiving any message resets our idle timeout.
            break;
        }
        default: {
            LOG_INFO("Received unexpected ChatServer message type: %u",
                     static_cast<unsigned int>(messageType));
            disconnectWithError(
                asio::error::make_error_code(asio::error::invalid_argument));
            break;
        }
    }
}

void ChatServiceEndpoint::cancelTimer(asio::steady_timer& timer)
{
    try {
        timer.cancel();
    } catch (const asio::system_error& e) {
        LOG_INFO("Failed to cancel ChatServer timer: %s",
                 e.code().message().c_str());
    }
}

asio::steady_timer::duration
    ChatServiceEndpoint::toTimerDuration(double seconds)
{
    return std::chrono::duration_cast<asio::steady_timer::duration>(
        std::chrono::duration<double>(seconds));
}

} // namespace WorldServer
} // namespace AM
