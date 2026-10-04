#include "AccountServiceEndpoint.h"
#include "Config.h"
#include "Log.h"
#include "asio/error.hpp"
#include <functional>
#include <utility>

namespace AM
{
namespace AccountServer
{

AccountServiceEndpoint::AccountServiceEndpoint(
    asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
    asio::thread_pool& inDatabasePool, Database& inDatabase)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, connectionRegistry(Config::MAX_SERVICE_CONNECTIONS)
, acceptor(
      inIoContext,
      asio::ip::tcp::endpoint{asio::ip::tcp::v4(),
                              Config::SERVER_SERVICE_PORT},
      std::bind_front(&AccountServiceEndpoint::connectionFactory, this),
      std::bind_front(&AccountServiceEndpoint::onConnectionAccepted, this),
      std::bind_front(&AccountServiceEndpoint::onAcceptorError, this))
, messageProcessor{
      inIoContext, inDatabasePool, inDatabase,
      std::bind_front(&AccountServiceEndpoint::sendMessage, this),
      std::bind_front(&AccountServiceEndpoint::disconnect, this)}
, worldServerHandle{NULL_CONNECTION_HANDLE}
{
}

void AccountServiceEndpoint::start()
{
    acceptor.start();
}

std::shared_ptr<AccountServiceEndpoint::Connection>
    AccountServiceEndpoint::connectionFactory(asio::ip::tcp::socket socket)
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
        .maxReadPayloadSize{Config::MAX_READ_PAYLOAD_SIZE},
        .maxWritePayloadSize{Config::MAX_WRITE_PAYLOAD_SIZE}};

    return std::make_shared<Connection>(std::move(socket), sslContext,
                                        connectionConfig);
}

void AccountServiceEndpoint::onConnectionAccepted(
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
        [this, handle = *handle](AccountServiceMessageType type,
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

void AccountServiceEndpoint::onAcceptorError(const asio::error_code& error)
{
    LOG_INFO("Account service acceptor error: %s", error.message().c_str());
}

void AccountServiceEndpoint::onConnectionReady(ConnectionHandle handle)
{
    // If there's an existing world server connection, it must be stale (we
    // only support 1 world server). Replace it.
    if (worldServerHandle) {
        LOG_INFO("New world server connection replaced an existing one.");
        ConnectionHandle oldHandle{worldServerHandle};
        worldServerHandle = NULL_CONNECTION_HANDLE;
        disconnect(oldHandle,
                   asio::error::make_error_code(asio::error::shut_down));
    }

    worldServerHandle = handle;
    LOG_INFO("World server connected.");
}

void AccountServiceEndpoint::onConnectionDisconnected(
    ConnectionHandle handle, const asio::error_code& error)
{
    if (handle == worldServerHandle) {
        LOG_INFO("World server disconnected: %s", error.message().c_str());
        worldServerHandle = NULL_CONNECTION_HANDLE;
    }
    else {
        LOG_INFO("Service connection disconnected: %s",
                 error.message().c_str());
    }

    connectionRegistry.erase(handle);
}

void AccountServiceEndpoint::onMessageReceived(
    ConnectionHandle handle, AccountServiceMessageType messageType,
    std::span<const Uint8> messageBuffer)
{
    messageProcessor.processReceivedMessage(handle, messageType, messageBuffer);
}

void AccountServiceEndpoint::sendMessage(ConnectionHandle handle,
                                         BinaryBufferSharedPtr message)
{
    if (std::shared_ptr<Connection> connection{
            connectionRegistry.find(handle)}) {
        connection->sendFramed(std::move(message));
    }
}

void AccountServiceEndpoint::disconnect(ConnectionHandle handle,
                                        const asio::error_code& error)
{
    if (std::shared_ptr<Connection> connection{
            connectionRegistry.find(handle)}) {
        connection->disconnect(error);
    }
}

} // end namespace AccountServer
} // end namespace AM
