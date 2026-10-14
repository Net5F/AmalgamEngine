#pragma once

#include "ChatServiceMessageType.h"
#include "SimpleConnection.h"
#include "ConnectionRegistry.h"
#include "Acceptor2.h"
#include "ServiceMessageProcessor.h"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include <memory>
#include <span>

namespace AM
{
namespace ChatServer
{

/**
 * Manages communication with our other internal services (world server).
 *
 * The world server uses this connection to tell us where players are, so we
 * can route local chat.
 *
 * Peers are authenticated through mutual TLS: the given sslContext must
 * require a client certificate and verify it against the world server's pin.
 * Because of this, any connection that completes its handshake is trusted.
 *
 * Only 1 world server is supported. If a new world server connection
 * completes its handshake while another is active, the old one is assumed to
 * be stale and is disconnected.
 *
 * The world server holds this connection open, sending periodic heartbeats
 * (which we respond to). If we stop receiving messages, we assume the world
 * server is gone and disconnect on timeout. The world server is expected to
 * reconnect.
 */
class ChatServiceEndpoint
{
public:
    ChatServiceEndpoint(asio::io_context& inIoContext,
                        asio::ssl::context& inSSLContext);

    /**
     * Starts our acceptor.
     */
    void start();

private:
    using Connection = SimpleConnection<ChatServiceMessageType>;

    // Acceptor callbacks
    std::shared_ptr<Connection> connectionFactory(asio::ip::tcp::socket socket);
    void onConnectionAccepted(std::shared_ptr<Connection> connection);
    void onAcceptorError(const asio::error_code& error);

    // Connection callbacks
    void onConnectionReady(ConnectionHandle handle);
    void onConnectionDisconnected(ConnectionHandle handle,
                                  const asio::error_code& error);
    void onMessageReceived(ConnectionHandle handle,
                           ChatServiceMessageType messageType,
                           std::span<const Uint8> messageBuffer);

    // ServiceMessageProcessor callbacks
    void sendMessage(ConnectionHandle handle, BinaryBufferSharedPtr message);
    void disconnect(ConnectionHandle handle, const asio::error_code& error);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;

    ConnectionRegistry<Connection> connectionRegistry;

    Acceptor<Connection> acceptor;

    ServiceMessageProcessor messageProcessor;

    /** The currently active world server connection. Null if none is
        connected. */
    ConnectionHandle worldServerHandle;
};

} // End namespace ChatServer
} // End namespace AM
