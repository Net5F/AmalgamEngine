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
    asio::thread_pool& inDatabasePool, Database& inDatabase,
    const ServicePins& inServicePins)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, connectionRegistry(Config::MAX_SERVICE_CONNECTIONS)
, acceptor(
      inIoContext,
      asio::ip::tcp::endpoint{asio::ip::tcp::v4(), Config::SERVER_SERVICE_PORT},
      std::bind_front(&AccountServiceEndpoint::connectionFactory, this),
      std::bind_front(&AccountServiceEndpoint::onConnectionAccepted, this),
      std::bind_front(&AccountServiceEndpoint::onAcceptorError, this))
, messageProcessor{inIoContext, inDatabasePool, inDatabase,
                   std::bind_front(&AccountServiceEndpoint::sendMessage, this),
                   std::bind_front(&AccountServiceEndpoint::disconnect, this)}
, servicePins{inServicePins}
, serviceHandles{}
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
        [this, handle = *handle](AM::Connection& readyConnection) {
            onConnectionReady(handle, readyConnection);
        },
        [this, handle = *handle](const asio::error_code& error) {
            onConnectionDisconnected(handle, error);
        });
}

void AccountServiceEndpoint::onAcceptorError(const asio::error_code& error)
{
    LOG_INFO("Account service acceptor error: %s", error.message().c_str());
}

void AccountServiceEndpoint::onConnectionReady(ConnectionHandle handle,
                                               AM::Connection& connection)
{
    // Figure out which service this is.
    // Note: The handshake already verified that the peer matches one of our
    //       pins, so this should never fail.
    std::optional<ServiceTicketAudience> service{identifyService(connection)};
    if (!service) {
        LOG_ERROR("Failed to identify service connection.");
        connection.disconnect(
            asio::error::make_error_code(asio::error::access_denied));
        return;
    }
    const char* serviceName{getServiceTicketAudienceName(*service)};

    // If there's an existing connection for this service, it must be stale
    // (we only support 1 per service). Replace it.
    ConnectionHandle& serviceHandle{
        serviceHandles[static_cast<std::size_t>(*service)]};
    if (serviceHandle) {
        LOG_INFO("New %s connection replaced an existing one.", serviceName);
        ConnectionHandle oldHandle{serviceHandle};
        serviceHandle = NULL_CONNECTION_HANDLE;
        disconnect(oldHandle,
                   asio::error::make_error_code(asio::error::shut_down));
    }

    serviceHandle = handle;
    LOG_INFO("%s connected.", serviceName);
}

void AccountServiceEndpoint::onConnectionDisconnected(
    ConnectionHandle handle, const asio::error_code& error)
{
    if (std::optional<ServiceTicketAudience> service{findService(handle)}) {
        LOG_INFO("%s disconnected: %s", getServiceTicketAudienceName(*service),
                 error.message().c_str());
        serviceHandles[static_cast<std::size_t>(*service)]
            = NULL_CONNECTION_HANDLE;
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
    // Only accept messages from active service connections.
    // Note: This should only fail if the connection was replaced and is in
    //       the process of disconnecting.
    std::optional<ServiceTicketAudience> service{findService(handle)};
    if (!service) {
        disconnect(handle,
                   asio::error::make_error_code(asio::error::access_denied));
        return;
    }

    messageProcessor.processReceivedMessage(handle, *service, messageType,
                                            messageBuffer);
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

std::optional<ServiceTicketAudience>
    AccountServiceEndpoint::identifyService(AM::Connection& connection)
{
    std::optional<TlsHelpers::CertificatePin> peerPin{
        TlsHelpers::getPeerCertificatePin(connection.nativeHandle())};
    if (!peerPin) {
        return {};
    }

    for (std::size_t i{0}; i < servicePins.size(); ++i) {
        if (servicePins[i] == *peerPin) {
            return static_cast<ServiceTicketAudience>(i);
        }
    }

    return {};
}

std::optional<ServiceTicketAudience>
    AccountServiceEndpoint::findService(ConnectionHandle handle)
{
    for (std::size_t i{0}; i < serviceHandles.size(); ++i) {
        if (serviceHandles[i] && (serviceHandles[i] == handle)) {
            return static_cast<ServiceTicketAudience>(i);
        }
    }

    return {};
}

} // End namespace AccountServer
} // End namespace AM
