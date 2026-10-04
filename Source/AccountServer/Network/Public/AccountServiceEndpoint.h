#pragma once

#include "AccountServiceMessageType.h"
#include "SimpleConnection.h"
#include "ConnectionRegistry.h"
#include "Acceptor2.h"
#include "ServiceMessageProcessor.h"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include "asio/thread_pool.hpp"
#include <memory>
#include <span>

namespace AM
{
namespace AccountServer
{
class Database;

/**
 * Manages communication with our other internal services (world server, chat
 * server).
 *
 * Services use this connection to validate the tickets that clients present
 * to them when logging in.
 *
 * Peers are authenticated through mutual TLS: the given sslContext must
 * require a client certificate and verify it against the service's pin.
 * Because of this, any connection that completes its handshake is trusted.
 *
 * Only 1 world server is supported. If a new world server connection
 * completes its handshake while another is active, the old one is assumed to
 * be stale and is disconnected.
 *
 * This connection stays open as long as messages are flowing, and disconnects
 * on timeout. The world server is expected to reconnect as needed.
 */
class AccountServiceEndpoint
{
public:
    AccountServiceEndpoint(asio::io_context& inIoContext,
                           asio::ssl::context& inSSLContext,
                           asio::thread_pool& inDatabasePool,
                           Database& inDatabase);

    /**
     * Starts our acceptor.
     */
    void start();

private:
    using Connection = SimpleConnection<AccountServiceMessageType>;

    // Acceptor callbacks
    std::shared_ptr<Connection> connectionFactory(asio::ip::tcp::socket socket);
    void onConnectionAccepted(std::shared_ptr<Connection> connection);
    void onAcceptorError(const asio::error_code& error);

    // Connection callbacks
    void onConnectionReady(ConnectionHandle handle);
    void onConnectionDisconnected(ConnectionHandle handle,
                                  const asio::error_code& error);
    void onMessageReceived(ConnectionHandle handle,
                           AccountServiceMessageType messageType,
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

} // End namespace AccountServer
} // End namespace AM
