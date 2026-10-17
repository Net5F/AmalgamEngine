#pragma once

#include "ChatClientMessageType.h"
#include "SimpleConnection.h"
#include "ConnectionRegistry.h"
#include "Acceptor2.h"
#include "ClientMessageProcessor.h"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include <memory>
#include <span>

namespace AM
{
class AccountServiceEndpoint;

namespace ChatServer
{
class PlayerLocator;

/**
 * Manages communication with clients (ran by users).
 *
 * Unlike AccountServer, clients hold this connection open for as long as
 * they're logged in, sending periodic heartbeats (which we respond to). If we
 * stop receiving messages, we assume the client is gone and disconnect on
 * timeout.
 */
class ChatClientEndpoint
{
public:
    ChatClientEndpoint(asio::io_context& inIoContext,
                       asio::ssl::context& inSSLContext,
                       AccountServiceEndpoint& inAccountEndpoint,
                       PlayerLocator& inPlayerLocator);

    /**
     * Starts our acceptor.
     */
    void start();

private:
    using Connection = SimpleConnection<ChatClientMessageType>;

    // Acceptor callbacks
    std::shared_ptr<Connection> connectionFactory(asio::ip::tcp::socket socket);
    void onConnectionAccepted(std::shared_ptr<Connection> connection);
    void onAcceptorError(const asio::error_code& error);

    // Connection callbacks
    void onConnectionDisconnected(ConnectionHandle handle,
                                  const asio::error_code& error);
    void onMessageReceived(ConnectionHandle handle,
                           ChatClientMessageType messageType,
                           std::span<const Uint8> messageBuffer);

    // ClientMessageProcessor callbacks
    void sendMessage(ConnectionHandle handle, BinaryBufferSharedPtr message);
    void disconnect(ConnectionHandle handle, const asio::error_code& error);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;

    ConnectionRegistry<Connection> connectionRegistry;

    Acceptor<Connection> acceptor;

    ClientMessageProcessor messageProcessor;
};

} // End namespace ChatServer
} // End namespace AM
