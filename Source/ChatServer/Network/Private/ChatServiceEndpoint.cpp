#include "ChatServiceEndpoint.h"
#include "Config.h"
#include "Log.h"
#include "asio/error.hpp"
#include <functional>
#include <utility>

namespace AM
{
namespace ChatServer
{

ChatServiceEndpoint::ChatServiceEndpoint(asio::io_context& inIoContext,
                                         asio::ssl::context& inSSLContext)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, connectionRegistry(Config::MAX_SERVICE_CONNECTIONS)
, acceptor(
      inIoContext,
      asio::ip::tcp::endpoint{asio::ip::tcp::v4(), Config::SERVER_SERVICE_PORT},
      std::bind_front(&ChatServiceEndpoint::connectionFactory, this),
      std::bind_front(&ChatServiceEndpoint::onConnectionAccepted, this),
      std::bind_front(&ChatServiceEndpoint::onAcceptorError, this))
, messageProcessor{std::bind_front(&ChatServiceEndpoint::sendMessage, this),
                   std::bind_front(&ChatServiceEndpoint::disconnect, this)}
, worldServerHandle{NULL_CONNECTION_HANDLE}
{
}

void ChatServiceEndpoint::start()
{
    acceptor.start();
}

std::shared_ptr<ChatServiceEndpoint::Connection>
    ChatServiceEndpoint::connectionFactory(asio::ip::tcp::socket socket)
{
    // Configure and create a new connection.
    TlsTransport::ClassConfig tlsTransportConfig{
        .handshakeTimeoutS{Config::CONNECTING_TIMEOUT_S},
        .maxQueuedWriteBytes{Config::SERVICE_MAX_QUEUED_WRITE_BYTES},
        .maxQueuedMessages{Config::SERVICE_MAX_QUEUED_WRITES}};

    Connection::ClassConfig connectionConfig{
        .tlsTransportConfig{tlsTransportConfig},
        .partialReceiveTimeoutS{Config::PARTIAL_RECEIVE_TIMEOUT_S},
        .idleTimeoutS{Config::SERVICE_IDLE_TIMEOUT_S},
        .maxReadPayloadSize{Config::SERVICE_MAX_READ_PAYLOAD_SIZE},
        .maxWritePayloadSize{Config::SERVICE_MAX_WRITE_PAYLOAD_SIZE}};

    return std::make_shared<Connection>(std::move(socket), sslContext,
                                        connectionConfig);
}

void ChatServiceEndpoint::onConnectionAccepted(
    std::shared_ptr<Connection> connection)
{
    // Try to add the connection to the registry.
    std::optional<ConnectionHandle> handle{connectionRegistry.add(connection)};
    if (!handle) {
        LOG_INFO("Rejected service connection: Already at maximum "
                 "connections.");
        connection->close();
        return;
    }

    // Successfully added. Register callbacks and start the handshake process.
    // Note: The peer's certificate is verified during the handshake, so
    //       messages won't be received until the peer is authenticated.
    connection->setMessageCallback(
        [this, handle = *handle](ChatServiceMessageType type,
                                 std::span<const Uint8> payload) {
            onMessageReceived(handle, type, payload);
        });

    connection->startServer(
        [this, handle = *handle](AM::Connection&) {
            onConnectionReady(handle);
        },
        [this, handle = *handle](const asio::error_code& error) {
            onConnectionDisconnected(handle, error);
        });
}

void ChatServiceEndpoint::onAcceptorError(const asio::error_code& error)
{
    LOG_INFO("Chat service acceptor error: %s", error.message().c_str());
}

void ChatServiceEndpoint::onConnectionReady(ConnectionHandle handle)
{
    // If there's an existing world server connection, it must be stale (we
    // only support 1 world server). Replace it.
    if (worldServerHandle) {
        LOG_INFO("New WorldServer connection replaced an existing one.");
        ConnectionHandle oldHandle{worldServerHandle};
        worldServerHandle = NULL_CONNECTION_HANDLE;
        disconnect(oldHandle,
                   asio::error::make_error_code(asio::error::shut_down));
    }

    worldServerHandle = handle;
    LOG_INFO("WorldServer connected.");
}

void ChatServiceEndpoint::onConnectionDisconnected(
    ConnectionHandle handle, const asio::error_code& error)
{
    if (handle == worldServerHandle) {
        LOG_INFO("WorldServer disconnected: %s", error.message().c_str());
        worldServerHandle = NULL_CONNECTION_HANDLE;
    }
    else {
        LOG_INFO("Service connection disconnected: %s",
                 error.message().c_str());
    }

    connectionRegistry.erase(handle);
}

void ChatServiceEndpoint::onMessageReceived(
    ConnectionHandle handle, ChatServiceMessageType messageType,
    std::span<const Uint8> messageBuffer)
{
    // Only accept messages from the active world server connection.
    // Note: This should only fail if the connection was replaced and is in
    //       the process of disconnecting.
    if (!worldServerHandle || (handle != worldServerHandle)) {
        disconnect(handle,
                   asio::error::make_error_code(asio::error::access_denied));
        return;
    }

    messageProcessor.processReceivedMessage(handle, messageType, messageBuffer);
}

void ChatServiceEndpoint::sendMessage(ConnectionHandle handle,
                                      BinaryBufferSharedPtr message)
{
    if (std::shared_ptr<Connection> connection{
            connectionRegistry.find(handle)}) {
        connection->sendFramed(std::move(message));
    }
}

void ChatServiceEndpoint::disconnect(ConnectionHandle handle,
                                     const asio::error_code& error)
{
    if (std::shared_ptr<Connection> connection{
            connectionRegistry.find(handle)}) {
        connection->disconnect(error);
    }
}

} // End namespace ChatServer
} // End namespace AM
