#include "ChatClientEndpoint.h"
#include "MessageProcessorContext.h"
#include "UserConfig.h"
#include "ChatConnectionRequest.h"
#include "ChatConnectionResponse.h"
#include "ChatError.h"
#include "ChatHeartbeat.h"
#include "ChatMessage.h"
#include "Deserialize.h"
#include "QueuedEvents.h"
#include "SimpleMessageFramer.h"
#include "ByteTools.h"
#include "asio/error.hpp"
#include "asio/ip/address.hpp"
#include "asio/system_error.hpp"
#include <openssl/crypto.h>
#include <chrono>
#include <limits>

namespace AM
{
namespace Client
{

ChatClientEndpoint::ChatClientEndpoint(
    asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
    const MessageProcessorContext& inMessageProcessorContext)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, networkEventDispatcher{inMessageProcessorContext.networkEventDispatcher}
, connectionTimer{inIoContext}
, heartbeatTimer{inIoContext}
, connectingSocket{}
, connection{}
, pendingTicket{}
, connectionState{ConnectionState::Disconnected}
, currentConnectionID{0}
, nextConnectionID{1}
{
}

ChatClientEndpoint::~ChatClientEndpoint()
{
    closeOnIOThread();
}

Uint32 ChatClientEndpoint::connect(
    const std::array<Uint8, SERVICE_TICKET_BYTES>& ticket)
{
    Uint32 connectionID{nextConnectionID++};
    asio::post(ioContext, [this, connectionID, ticketCopy = ticket]() mutable {
        connectOnIOThread(connectionID, ticketCopy);
        OPENSSL_cleanse(ticketCopy.data(), ticketCopy.size());
    });

    return connectionID;
}

void ChatClientEndpoint::disconnect()
{
    asio::post(ioContext, [this]() { closeOnIOThread(); });
}

void ChatClientEndpoint::connectOnIOThread(
    Uint32 connectionID, std::array<Uint8, SERVICE_TICKET_BYTES> ticket)
{
    // If a connection is already open, close it.
    closeOnIOThread();

    currentConnectionID = connectionID;
    connectionState = ConnectionState::Connecting;
    pendingTicket = ticket;
    OPENSSL_cleanse(ticket.data(), ticket.size());

    UserConfig::ServerAddress serverAddress{
        UserConfig::get().getChatServerAddress()};
    asio::error_code error{};
    asio::ip::address address{asio::ip::make_address(serverAddress.IP, error)};
    if (error || (serverAddress.port > std::numeric_limits<Uint16>::max())) {
        if (!error) {
            error = asio::error::make_error_code(asio::error::invalid_argument);
        }
        handleConnectionFailure(connectionID, error);
        return;
    }

    connectingSocket = std::make_shared<asio::ip::tcp::socket>(ioContext);
    std::shared_ptr<asio::ip::tcp::socket> socket{connectingSocket};

    // Attempt to connect, with a timeout.
    // Note: The timeout also covers the TLS handshake (see onSocketConnected).
    try {
        connectionTimer.expires_after(
            std::chrono::duration_cast<asio::steady_timer::duration>(
                std::chrono::duration<double>(CONNECTING_TIMEOUT_S)));
        connectionTimer.async_wait([this, socket, connectionID](
                                       const asio::error_code& timerError) {
            if (timerError || (connectionID != currentConnectionID)
                || (connectionState != ConnectionState::Connecting)) {
                return;
            }

            asio::error_code ignoredError{};
            socket->cancel(ignoredError);
            socket->close(ignoredError);
            handleConnectionFailure(connectionID, asio::error::make_error_code(
                                                      asio::error::timed_out));
        });

        socket->async_connect(
            asio::ip::tcp::endpoint{
                address, static_cast<unsigned short>(serverAddress.port)},
            [this, socket, connectionID](const asio::error_code& connectError) {
                onSocketConnected(socket, connectionID, connectError);
            });
    } catch (const asio::system_error& e) {
        handleConnectionFailure(connectionID, e.code());
    }
}

void ChatClientEndpoint::closeOnIOThread()
{
    // Note: Bumping the ID makes any late callbacks for this connection
    //       ignore themselves.
    ++currentConnectionID;
    connectionState = ConnectionState::Disconnected;
    cancelTimer(connectionTimer);
    cancelTimer(heartbeatTimer);
    OPENSSL_cleanse(pendingTicket.data(), pendingTicket.size());

    asio::error_code ignoredError{};
    if (connectingSocket) {
        connectingSocket->cancel(ignoredError);
        connectingSocket->close(ignoredError);
        connectingSocket.reset();
    }
    if (connection) {
        connection->close();
        connection.reset();
    }
}

void ChatClientEndpoint::handleConnectionFailure(Uint32 connectionID,
                                                 const asio::error_code& error)
{
    if ((connectionID != currentConnectionID)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    LOG_INFO("ChatServer connection failed: %s", error.message().c_str());
    closeOnIOThread();
    emitConnectionEvent(connectionID,
                        ChatConnectionEvent::Type::ConnectionFailed);
}

void ChatClientEndpoint::disconnectWithError(const asio::error_code& error)
{
    if (connection) {
        connection->disconnect(error);
    }
}

void ChatClientEndpoint::cancelTimer(asio::steady_timer& timer)
{
    try {
        timer.cancel();
    } catch (const asio::system_error& e) {
        LOG_INFO("Failed to cancel ChatServer timer: %s",
                 e.code().message().c_str());
    }
}

void ChatClientEndpoint::startHeartbeatTimer(Uint32 connectionID)
{
    try {
        heartbeatTimer.expires_after(
            std::chrono::duration_cast<asio::steady_timer::duration>(
                std::chrono::duration<double>(HEARTBEAT_INTERVAL_S)));
        heartbeatTimer.async_wait(
            [this, connectionID](const asio::error_code& timerError) {
                if (timerError || (connectionID != currentConnectionID)
                    || (connectionState != ConnectionState::Authenticated)) {
                    return;
                }

                sendOnIOThread(ChatHeartbeat{});
                startHeartbeatTimer(connectionID);
            });
    } catch (const asio::system_error& e) {
        LOG_INFO("Failed to start ChatServer heartbeat timer: %s",
                 e.code().message().c_str());
        disconnectWithError(e.code());
    }
}

void ChatClientEndpoint::onSocketConnected(
    std::shared_ptr<asio::ip::tcp::socket> socket, Uint32 connectionID,
    const asio::error_code& error)
{
    if ((connectionID != currentConnectionID)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    if (error) {
        handleConnectionFailure(connectionID, error);
        return;
    }

    // Socket successfully connected. Configure and create the new connection.
    // Note: We leave connectionTimer running, so it also bounds the TLS
    //       handshake.
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
        [this](ChatClientMessageType messageType,
               std::span<const Uint8> messageBuffer) {
            onMessageReceived(messageType, messageBuffer);
        });
    connection->startClient(
        [this, connectionID](AM::Connection&) {
            onConnectionReady(connectionID);
        },
        [this, connectionID](const asio::error_code& disconnectError) {
            onConnectionDisconnected(connectionID, disconnectError);
        });
}

void ChatClientEndpoint::onConnectionReady(Uint32 connectionID)
{
    if ((connectionID != currentConnectionID)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    // Present our ticket. ChatServer will respond with a
    // ChatConnectionResponse.
    // Note: From here, our idle timeout bounds how long we'll wait.
    cancelTimer(connectionTimer);
    connectionState = ConnectionState::Authenticating;
    // Note: We frame the request ourselves so that we can wipe the ticket from
    //       every buffer that holds it.
    ChatConnectionRequest connectionRequest{pendingTicket};
    BinaryBufferSharedPtr framedRequest{
        SimpleMessageFramer<ChatClientMessageType>{MAX_WRITE_PAYLOAD_SIZE}
            .frameMessage(connectionRequest)};
    OPENSSL_cleanse(connectionRequest.ticket.data(),
                    connectionRequest.ticket.size());
    OPENSSL_cleanse(pendingTicket.data(), pendingTicket.size());
    if (!framedRequest
        || !connection->sendFramed(
            ByteTools::makeWipedOnRelease(std::move(*framedRequest)))) {
        LOG_INFO("Failed to queue ChatServer connection request.");
    }
}

void ChatClientEndpoint::onConnectionDisconnected(Uint32 connectionID,
                                                  const asio::error_code& error)
{
    if (connectionID != currentConnectionID) {
        return;
    }

    ConnectionState previousState{connectionState};
    LOG_INFO("ChatServer disconnected: %s", error.message().c_str());
    closeOnIOThread();

    if (previousState == ConnectionState::Authenticated) {
        emitConnectionEvent(connectionID,
                            ChatConnectionEvent::Type::Disconnected);
    }
    else if (previousState != ConnectionState::Disconnected) {
        emitConnectionEvent(connectionID,
                            ChatConnectionEvent::Type::ConnectionFailed);
    }
}

void ChatClientEndpoint::onMessageReceived(ChatClientMessageType messageType,
                                           std::span<const Uint8> messageBuffer)
{
    // ChatServer only sends a ChatConnectionResponse while we're
    // authenticating, and only sends the others after we're authenticated.
    bool isResponse{messageType
                    == ChatClientMessageType::ChatConnectionResponse};
    bool messageAllowed{
        isResponse ? (connectionState == ConnectionState::Authenticating)
                   : (connectionState == ConnectionState::Authenticated)};
    if (!messageAllowed) {
        LOG_INFO("Received ChatServer message in an invalid state. Type: %u",
                 static_cast<unsigned int>(messageType));
        disconnectWithError(
            asio::error::make_error_code(asio::error::invalid_argument));
        return;
    }

    switch (messageType) {
        case ChatClientMessageType::ChatConnectionResponse: {
            ChatConnectionResponse response{};
            if (!deserialize(messageBuffer, response)) {
                return;
            }

            const Uint32 connectionID{currentConnectionID};
            if (response.result == ChatConnectionResponse::Success) {
                connectionState = ConnectionState::Authenticated;
                startHeartbeatTimer(connectionID);
                emitConnectionEvent(connectionID,
                                    ChatConnectionEvent::Type::Authenticated);
            }
            else {
                // ChatServer expects us to close the connection.
                closeOnIOThread();
                emitConnectionEvent(
                    connectionID,
                    (response.result == ChatConnectionResponse::InvalidTicket)
                        ? ChatConnectionEvent::Type::InvalidTicket
                        : ChatConnectionEvent::Type::Rejected);
            }
            break;
        }
        case ChatClientMessageType::ChatMessage: {
            ChatMessage chatMessage{};
            if (deserialize(messageBuffer, chatMessage)) {
                networkEventDispatcher.push<ChatMessage>(chatMessage);
            }
            break;
        }
        case ChatClientMessageType::ChatError: {
            ChatError chatError{};
            if (deserialize(messageBuffer, chatError)) {
                networkEventDispatcher.push<ChatError>(chatError);
            }
            break;
        }
        case ChatClientMessageType::ChatHeartbeatResponse: {
            // Receiving it already reset our idle timeout, nothing to do.
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

template<typename Message>
bool ChatClientEndpoint::deserialize(std::span<const Uint8> messageBuffer,
                                     Message& message)
{
    if (!Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                                 message)) {
        LOG_INFO("Failed to deserialize ChatServer message.");
        disconnectWithError(
            asio::error::make_error_code(asio::error::invalid_argument));
        return false;
    }

    return true;
}

void ChatClientEndpoint::emitConnectionEvent(Uint32 connectionID,
                                             ChatConnectionEvent::Type type)
{
    networkEventDispatcher.emplace<ChatConnectionEvent>(type, connectionID);
}

} // namespace Client
} // namespace AM
