#include "ChatClientEndpoint.h"
#include "Config.h"
#include "Log.h"
#include <functional>
#include <utility>

namespace AM
{
namespace ChatServer
{

ChatClientEndpoint::ChatClientEndpoint(
    asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
    AccountServiceEndpoint& inAccountEndpoint)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, connectionRegistry(Config::MAX_CLIENTS)
, acceptor(
      inIoContext,
      asio::ip::tcp::endpoint{asio::ip::tcp::v4(), Config::SERVER_CLIENT_PORT},
      std::bind_front(&ChatClientEndpoint::connectionFactory, this),
      std::bind_front(&ChatClientEndpoint::onConnectionAccepted, this),
      std::bind_front(&ChatClientEndpoint::onAcceptorError, this))
, messageProcessor{inAccountEndpoint,
                   std::bind_front(&ChatClientEndpoint::sendMessage, this),
                   std::bind_front(&ChatClientEndpoint::disconnect, this)}
{
}

void ChatClientEndpoint::start()
{
    acceptor.start();
}

std::shared_ptr<ChatClientEndpoint::Connection>
    ChatClientEndpoint::connectionFactory(asio::ip::tcp::socket socket)
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

void ChatClientEndpoint::onConnectionAccepted(
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
        [this, handle = *handle](ChatClientMessageType type,
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

void ChatClientEndpoint::onAcceptorError(const asio::error_code& error)
{
    LOG_INFO("Chat client acceptor error: %s", error.message().c_str());
}

void ChatClientEndpoint::onConnectionDisconnected(ConnectionHandle handle,
                                                  const asio::error_code& error)
{
    // Note: Timeouts are common (idle clients), and the OS's message for them
    //       is long, so we print a short message instead.
    if (error == asio::error::timed_out) {
        LOG_INFO("Chat client disconnected: Timeout.");
    }
    else {
        LOG_INFO("Chat client disconnected: %s", error.message().c_str());
    }
    messageProcessor.onClientDisconnected(handle);
    connectionRegistry.erase(handle);
}

void ChatClientEndpoint::onMessageReceived(ConnectionHandle handle,
                                           ChatClientMessageType messageType,
                                           std::span<const Uint8> messageBuffer)
{
    messageProcessor.processReceivedMessage(handle, messageType, messageBuffer);
}

void ChatClientEndpoint::sendMessage(ConnectionHandle handle,
                                     BinaryBufferSharedPtr message)
{
    if (std::shared_ptr<Connection> connection{
            connectionRegistry.find(handle)}) {
        connection->sendFramed(std::move(message));
    }
}

void ChatClientEndpoint::disconnect(ConnectionHandle handle,
                                    const asio::error_code& error)
{
    if (std::shared_ptr<Connection> connection{
            connectionRegistry.find(handle)}) {
        connection->disconnect(error);
    }
}

} // End namespace ChatServer
} // End namespace AM
