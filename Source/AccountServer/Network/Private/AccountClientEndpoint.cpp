#include "AccountClientEndpoint.h"
#include "Config.h"
#include "Log.h"
#include <functional>
#include <utility>

namespace AM
{
namespace AccountServer
{

AccountClientEndpoint::AccountClientEndpoint(
    asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
    asio::thread_pool& inDatabasePool, Database& inDatabase)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, connectionRegistry(Config::MAX_CLIENTS)
, acceptor(
      inIoContext,
      asio::ip::tcp::endpoint{asio::ip::tcp::v4(), Config::SERVER_CLIENT_PORT},
      std::bind_front(&AccountClientEndpoint::connectionFactory, this),
      std::bind_front(&AccountClientEndpoint::onConnectionAccepted, this),
      std::bind_front(&AccountClientEndpoint::onAcceptorError, this))
, messageProcessor{
      inIoContext, inDatabasePool, inDatabase,
      std::bind_front(&AccountClientEndpoint::sendMessage, this),
      std::bind_front(&AccountClientEndpoint::disconnect, this)}
{
}

void AccountClientEndpoint::start()
{
    acceptor.start();
}

std::shared_ptr<AccountClientEndpoint::Connection>
    AccountClientEndpoint::connectionFactory(asio::ip::tcp::socket socket)
{
    // Configure and create a new connection.
    TlsTransport::ClassConfig tlsTransportConfig{
        .handshakeTimeoutS{Config::CONNECTING_TIMEOUT_S},
        .maxQueuedWriteBytes{Config::MAX_QUEUED_WRITE_BYTES},
        .maxQueuedMessages{Config::MAX_QUEUED_WRITES}};

    Connection::ClassConfig connectionConfig{
        .tlsTransportConfig{tlsTransportConfig},
        .partialReceiveTimeoutS{Config::PARTIAL_RECEIVE_TIMEOUT_S},
        .idleTimeoutS{Config::IDLE_TIMEOUT_S},
        .maxReadPayloadSize{Config::MAX_READ_PAYLOAD_SIZE},
        .maxWritePayloadSize{Config::MAX_WRITE_PAYLOAD_SIZE}};

    return std::make_shared<Connection>(std::move(socket), sslContext,
                                        connectionConfig);
}

void AccountClientEndpoint::onConnectionAccepted(
    std::shared_ptr<Connection> connection)
{
    // Try to add the connection to the registry.
    std::optional<ConnectionHandle> handle{connectionRegistry.add(connection)};
    if (!handle) {
        connection->close();
        return;
    }

    // Successfully added. Register callbacks and start the handshake process.
    connection->setMessageCallback(
        [this, handle = *handle](AccountClientMessageType type,
                                 std::span<const Uint8> payload) {
            onMessageReceived(handle, type, payload);
        });

    connection->startServer(
        [](AM::Connection&) {
            // Don't need to do anything on connection ready.
        },
        [this, handle = *handle](const asio::error_code& error) {
            onConnectionDisconnected(handle, error);
        });
}

void AccountClientEndpoint::onAcceptorError(const asio::error_code& error)
{
    LOG_INFO("Account client acceptor error: %s", error.message().c_str());
}

void AccountClientEndpoint::onConnectionDisconnected(
    ConnectionHandle handle, const asio::error_code& error)
{
    LOG_INFO("Account client disconnected: %s", error.message().c_str());
    connectionRegistry.erase(handle);
}

void AccountClientEndpoint::onMessageReceived(
    ConnectionHandle handle, AccountClientMessageType messageType,
    std::span<const Uint8> messageBuffer)
{
    messageProcessor.processReceivedMessage(handle, messageType, messageBuffer);
}

void AccountClientEndpoint::sendMessage(ConnectionHandle handle,
                                        BinaryBufferSharedPtr message)
{
    if (std::shared_ptr<Connection> connection{
            connectionRegistry.find(handle)}) {
        connection->sendFramed(std::move(message));
    }
}

void AccountClientEndpoint::disconnect(ConnectionHandle handle,
                                       const asio::error_code& error)
{
    if (std::shared_ptr<Connection> connection{
            connectionRegistry.find(handle)}) {
        connection->disconnect(error);
    }
}

} // end namespace AccountServer
} // end namespace AM
