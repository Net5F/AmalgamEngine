#pragma once

#include "AccountClientMessageType.h"
#include "SimpleConnection.h"
#include "ConnectionRegistry.h"
#include "Acceptor2.h"
#include "ClientMessageProcessor.h"
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
 * Manages communication with clients (ran by users).
 */
class AccountClientEndpoint
{
public:
    AccountClientEndpoint(asio::io_context& inIoContext,
                          asio::ssl::context& inSSLContext,
                          asio::thread_pool& inDatabasePool,
                          Database& inDatabase);

    /**
     * Starts our acceptor.
     */
    void start();

private:
    using Connection = SimpleConnection<AccountClientMessageType>;

    // Acceptor callbacks
    std::shared_ptr<Connection> connectionFactory(asio::ip::tcp::socket socket);
    void onConnectionAccepted(std::shared_ptr<Connection> connection);
    void onAcceptorError(const asio::error_code& error);

    // Connection callbacks
    void onConnectionDisconnected(ConnectionHandle handle,
                                  const asio::error_code& error);
    void onMessageReceived(ConnectionHandle handle,
                           AccountClientMessageType messageType,
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

} // End namespace AccountServer
} // End namespace AM
