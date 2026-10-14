#pragma once

#include "AccountDefs.h"
#include "AccountServiceMessageType.h"
#include "SimpleConnection.h"
#include "ConnectionRegistry.h"
#include "Acceptor2.h"
#include "ServiceMessageProcessor.h"
#include "TlsHelpers.h"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include "asio/thread_pool.hpp"
#include <array>
#include <memory>
#include <optional>
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
 * require a client certificate and verify it against the services' pins.
 * Because of this, any connection that completes its handshake is trusted.
 * Once the handshake completes, we compare the peer's certificate against
 * each service's pin to identify which service it is. A service can only
 * consume tickets that were issued for it.
 *
 * Only 1 connection per service is supported. If a new connection for a
 * service completes its handshake while another is active, the old one is
 * assumed to be stale and is disconnected.
 *
 * Services hold this connection open, sending periodic heartbeats (which we
 * respond to). If we stop receiving messages, we assume the service is gone
 * and disconnect on timeout. Services are expected to reconnect.
 */
class AccountServiceEndpoint
{
public:
    /** The pin of each service that's allowed to connect, indexed by
        service. */
    using ServicePins
        = std::array<TlsHelpers::CertificatePin,
                     static_cast<std::size_t>(ServiceTicketAudience::Count)>;

    AccountServiceEndpoint(asio::io_context& inIoContext,
                           asio::ssl::context& inSSLContext,
                           asio::thread_pool& inDatabasePool,
                           Database& inDatabase,
                           const ServicePins& inServicePins);

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
    void onConnectionReady(ConnectionHandle handle, AM::Connection& connection);
    void onConnectionDisconnected(ConnectionHandle handle,
                                  const asio::error_code& error);
    void onMessageReceived(ConnectionHandle handle,
                           AccountServiceMessageType messageType,
                           std::span<const Uint8> messageBuffer);

    // ServiceMessageProcessor callbacks
    void sendMessage(ConnectionHandle handle, BinaryBufferSharedPtr message);
    void disconnect(ConnectionHandle handle, const asio::error_code& error);

    /**
     * Returns the service that the given connection's peer certificate
     * belongs to, or null if it doesn't match any service.
     */
    std::optional<ServiceTicketAudience>
        identifyService(AM::Connection& connection);

    /**
     * Returns the service that the given connection belongs to, or null if
     * it isn't an active service connection.
     */
    std::optional<ServiceTicketAudience>
        findService(ConnectionHandle handle);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;

    ConnectionRegistry<Connection> connectionRegistry;

    Acceptor<Connection> acceptor;

    ServiceMessageProcessor messageProcessor;

    /** The pin of each service that's allowed to connect. */
    ServicePins servicePins;

    /** The currently active connection for each service, indexed by service.
        Null if that service isn't connected. */
    std::array<ConnectionHandle,
               static_cast<std::size_t>(ServiceTicketAudience::Count)>
        serviceHandles;
};

} // End namespace AccountServer
} // End namespace AM
