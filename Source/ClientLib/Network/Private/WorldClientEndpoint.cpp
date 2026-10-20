#include "WorldClientEndpoint.h"
#include "MessageProcessorContext.h"
#include "QueuedEvents.h"
#include "Heartbeat.h"
#include "ConnectionRequest.h"
#include "ConnectionError.h"
#include "Config.h"
#include "UserConfig.h"
#include "NetworkStats.h"
#include "ByteTools.h"
#include "IMessageProcessorExtension.h"
#include "asio/error.hpp"
#include "asio/ip/address.hpp"
#include "asio/system_error.hpp"
#include <openssl/crypto.h>
#include <chrono>
#include <future>
#include <limits>

namespace AM
{
namespace Client
{
WorldClientEndpoint::WorldClientEndpoint(
    asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
    const MessageProcessorContext& inMessageProcessorContext)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, networkEventDispatcher{inMessageProcessorContext.networkEventDispatcher}
, messageProcessor{inMessageProcessorContext}
, connectionTimer{inIoContext}
, connectingSocket{}
, connection{}
, pendingWorldTicket{}
, connectionState{ConnectionState::Disconnected}
, currentConnectionID{0}
, connectionActive{false}
, messagesSentSinceTick{0}
, currentTickPtr{nullptr}
, netstatsLoggingEnabled{true}
, ticksSinceNetstatsLog{0}
, tickAdjustment{0}
, adjustmentIteration{0}
, isApplyingTickAdjustment{false}
{
}

WorldClientEndpoint::~WorldClientEndpoint()
{
    // Note: Network stops the IO thread before destroying us, so we can close
    //       directly.
    closeOnIOThread();
}

void WorldClientEndpoint::connect(
    const std::array<Uint8, SERVICE_TICKET_BYTES>& worldTicket)
{
    if (connectionActive) {
        LOG_INFO("Attempted to connect while connected.");
        return;
    }

    connectionActive = true;
    asio::post(ioContext, [this, ticketCopy = worldTicket]() mutable {
        connectOnIOThread(ticketCopy);
        OPENSSL_cleanse(ticketCopy.data(), ticketCopy.size());
    });
}

void WorldClientEndpoint::disconnect()
{
    connectionActive = false;

    // Wait for the IO thread to close the connection. This guarantees that
    // nothing else will be pushed for it once we return.
    std::promise<void> closedPromise{};
    std::future<void> closedFuture{closedPromise.get_future()};
    asio::post(ioContext, [this, &closedPromise]() {
        closeOnIOThread();
        closedPromise.set_value();
    });
    closedFuture.wait();

    // The IO thread is done with this connection, reset our shared state.
    tickAdjustment = 0;
    adjustmentIteration = 0;
    isApplyingTickAdjustment = false;
    messagesSentSinceTick = 0;
    ticksSinceNetstatsLog = 0;
}

void WorldClientEndpoint::tick()
{
    if (!Config::RUN_OFFLINE && connectionActive) {
        // If the sim is running, send a heartbeat if we need to.
        if (*currentTickPtr != 0) {
            sendHeartbeatIfNecessary();
        }

        // If it's time to log our network statistics, do so.
        if (netstatsLoggingEnabled) {
            ticksSinceNetstatsLog++;
            if (ticksSinceNetstatsLog == TICKS_TILL_STATS_DUMP) {
                logNetworkStatistics();
                ticksSinceNetstatsLog = 0;
            }
        }
    }
}

Uint32 WorldClientEndpoint::getLastReceivedTick()
{
    return messageProcessor.getLastReceivedTick();
}

int WorldClientEndpoint::transferTickAdjustment()
{
    if (isApplyingTickAdjustment) {
        int currentAdjustment = tickAdjustment;
        if (currentAdjustment < 0) {
            // The sim can only freeze for 1 tick at a time, transfer 1 from
            // tickAdjustment.
            tickAdjustment += 1;
            return -1;
        }
        else if (currentAdjustment > 0) {
            // The sim can process multiple iterations to catch up, transfer
            // all of tickAdjustment.
            tickAdjustment -= currentAdjustment;
            return currentAdjustment;
        }
        else {
            // We finished applying the adjustment, increment the iteration.
            adjustmentIteration++;
            isApplyingTickAdjustment = false;
            return 0;
        }
    }
    else {
        return 0;
    }
}

void WorldClientEndpoint::registerCurrentTickPtr(
    const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void WorldClientEndpoint::setNetstatsLoggingEnabled(
    bool inNetstatsLoggingEnabled)
{
    netstatsLoggingEnabled = inNetstatsLoggingEnabled;
}

void WorldClientEndpoint::setMessageProcessorExtension(
    IMessageProcessorExtension* extension)
{
    messageProcessor.setExtension(extension);
}

void WorldClientEndpoint::connectOnIOThread(
    const std::array<Uint8, SERVICE_TICKET_BYTES>& worldTicket)
{
    // If a connection is already open, close it.
    closeOnIOThread();

    const Uint32 connectionID{currentConnectionID};
    connectionState = ConnectionState::Connecting;
    pendingWorldTicket = worldTicket;

    UserConfig::ServerAddress serverAddress{
        UserConfig::get().getWorldServerAddress()};
    asio::error_code error{};
    asio::ip::address address{asio::ip::make_address(serverAddress.IP, error)};
    if (error || (serverAddress.port > std::numeric_limits<Uint16>::max())) {
        if (!error) {
            error = asio::error::make_error_code(asio::error::invalid_argument);
        }
        handleConnectionFailure(connectionID, error);
        return;
    }

    connectingSocket = std::make_shared<asio::ip::tcp::socket>(ioContext);
    std::shared_ptr<asio::ip::tcp::socket> socket{connectingSocket};

    // Attempt to connect, with a timeout.
    // Note: The timeout also covers the TLS handshake (see onSocketConnected).
    try {
        connectionTimer.expires_after(
            std::chrono::duration_cast<asio::steady_timer::duration>(
                std::chrono::duration<double>(CONNECTING_TIMEOUT_S)));
        connectionTimer.async_wait([this, socket, connectionID](
                                       const asio::error_code& timerError) {
            if (timerError || (connectionID != currentConnectionID)
                || (connectionState != ConnectionState::Connecting)) {
                return;
            }

            asio::error_code ignoredError{};
            socket->cancel(ignoredError);
            socket->close(ignoredError);
            handleConnectionFailure(connectionID, asio::error::make_error_code(
                                                      asio::error::timed_out));
        });

        socket->async_connect(
            asio::ip::tcp::endpoint{
                address, static_cast<unsigned short>(serverAddress.port)},
            [this, socket, connectionID](const asio::error_code& connectError) {
                onSocketConnected(socket, connectionID, connectError);
            });
    } catch (const asio::system_error& e) {
        handleConnectionFailure(connectionID, e.code());
    }
}

void WorldClientEndpoint::closeOnIOThread()
{
    // Note: Bumping the ID makes any late callbacks for this connection
    //       ignore themselves.
    ++currentConnectionID;
    connectionState = ConnectionState::Disconnected;
    cancelTimer(connectionTimer);
    OPENSSL_cleanse(pendingWorldTicket.data(), pendingWorldTicket.size());

    asio::error_code ignoredError{};
    if (connectingSocket) {
        connectingSocket->cancel(ignoredError);
        connectingSocket->close(ignoredError);
        connectingSocket.reset();
    }
    if (connection) {
        connection->close();
        connection.reset();
    }
}

void WorldClientEndpoint::handleConnectionFailure(Uint32 connectionID,
                                                  const asio::error_code& error)
{
    if ((connectionID != currentConnectionID)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    LOG_INFO("WorldServer connection failed: %s", error.message().c_str());
    closeOnIOThread();
    networkEventDispatcher.emplace<ConnectionError>(
        ConnectionError::Type::Failed);
}

void WorldClientEndpoint::cancelTimer(asio::steady_timer& timer)
{
    try {
        timer.cancel();
    } catch (const asio::system_error& e) {
        LOG_INFO("Failed to cancel WorldServer timer: %s",
                 e.code().message().c_str());
    }
}

void WorldClientEndpoint::onSocketConnected(
    std::shared_ptr<asio::ip::tcp::socket> socket, Uint32 connectionID,
    const asio::error_code& error)
{
    if ((connectionID != currentConnectionID)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    if (error) {
        handleConnectionFailure(connectionID, error);
        return;
    }

    // Disable Nagle's algorithm. Most of our messages are small and
    // latency-sensitive (e.g. inputs).
    asio::error_code optionError{};
    socket->set_option(asio::ip::tcp::no_delay{true}, optionError);
    if (optionError) {
        handleConnectionFailure(connectionID, optionError);
        return;
    }

    // Socket successfully connected. Configure and create the new connection.
    // Note: We leave connectionTimer running, so it also bounds the TLS
    //       handshake.
    TlsTransport::ClassConfig tlsTransportConfig{
        .handshakeTimeoutS{CONNECTING_TIMEOUT_S},
        .maxQueuedWriteBytes{MAX_QUEUED_WRITE_BYTES},
        .maxQueuedMessages{MAX_QUEUED_WRITES}};
    WorldConnection::ClassConfig connectionConfig{
        .tlsTransportConfig{tlsTransportConfig},
        .idleTimeoutS{IDLE_TIMEOUT_S}};

    connection = std::make_shared<WorldConnection>(
        std::move(*socket), sslContext, connectionConfig);
    connectingSocket.reset();

    connection->setBatchCallback(
        [this](const WorldBatchComposer::ReceivedBatch& batch) {
            onBatchReceived(batch);
        });
    connection->startClient(
        [this, connectionID](AM::Connection&) {
            onConnectionReady(connectionID);
        },
        [this, connectionID](const asio::error_code& disconnectError) {
            onConnectionDisconnected(connectionID, disconnectError);
        });
}

void WorldClientEndpoint::onConnectionReady(Uint32 connectionID)
{
    if ((connectionID != currentConnectionID)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    // Present our ticket. The server will validate it and send us a
    // ConnectionResponse.
    // Note: From here, ServerConnectionSystem's response timeout and our
    //       idle timeout bound how long we'll wait.
    cancelTimer(connectionTimer);
    connectionState = ConnectionState::Connected;
    ConnectionRequest connectionRequest{pendingWorldTicket};
    BinaryBufferSharedPtr framedRequest{WorldMessageFramer::frameMessage(
        connectionRequest, adjustmentIteration)};
    OPENSSL_cleanse(connectionRequest.ticket.data(),
                    connectionRequest.ticket.size());
    OPENSSL_cleanse(pendingWorldTicket.data(), pendingWorldTicket.size());
    if (!framedRequest) {
        return;
    }

    sendOnIOThread(ByteTools::makeWipedOnRelease(std::move(*framedRequest)));
}

void WorldClientEndpoint::onConnectionDisconnected(
    Uint32 connectionID, const asio::error_code& error)
{
    if (connectionID != currentConnectionID) {
        return;
    }

    ConnectionState previousState{connectionState};
    LOG_INFO("WorldServer disconnected: %s", error.message().c_str());
    closeOnIOThread();

    networkEventDispatcher.emplace<ConnectionError>(
        (previousState == ConnectionState::Connected)
            ? ConnectionError::Type::Disconnected
            : ConnectionError::Type::Failed);
}

void WorldClientEndpoint::onBatchReceived(
    const WorldBatchComposer::ReceivedBatch& batch)
{
    // Check if we need to adjust the tick offset.
    adjustIfNeeded(batch.tickAdjustment, batch.adjustmentIteration);

    // Process the messages.
    for (const WorldBatchComposer::ReceivedMessage& message : batch.messages) {
        messageProcessor.processReceivedMessage(
            message.type, message.payload.data(), message.payload.size());
    }

    // Record the number of received bytes.
    NetworkStats::recordBytesReceived(batch.wireSize);
}

void WorldClientEndpoint::sendOnIOThread(BinaryBufferSharedPtr message)
{
    if (!message) {
        return;
    }
    if ((connectionState != ConnectionState::Connected) || !connection) {
        LOG_INFO("Dropped a WorldServer message: Not connected.");
        return;
    }

    // Note: If the send fails, the connection will call our disconnect
    //       callback, which resets connection. We hold a reference so it
    //       outlives the call.
    std::shared_ptr<WorldConnection> activeConnection{connection};
    std::size_t messageSize{message->size()};
    if (activeConnection->sendFramed(std::move(message))) {
        NetworkStats::recordBytesSent(messageSize);
    }
    else {
        LOG_INFO("Failed to queue a WorldServer message.");
    }
}

void WorldClientEndpoint::sendHeartbeatIfNecessary()
{
    // If we haven't sent any relevant messages since the last tick.
    if (messagesSentSinceTick == 0) {
        // Send the heartbeat message.
        send<Heartbeat>({*currentTickPtr});
    }

    messagesSentSinceTick = 0;
}

void WorldClientEndpoint::adjustIfNeeded(Sint8 receivedTickAdj,
                                         Uint8 receivedAdjIteration)
{
    if (receivedTickAdj != 0) {
        Uint8 currentAdjIteration{adjustmentIteration};

        // If it's the current iteration and we aren't already applying it.
        if ((receivedAdjIteration == currentAdjIteration)
            && !isApplyingTickAdjustment) {
            // Set the adjustment to be applied.
            tickAdjustment += receivedTickAdj;
            isApplyingTickAdjustment = true;
            LOG_INFO("Received tick adjustment: %d, iteration: %u",
                     receivedTickAdj, receivedAdjIteration);
        }
        else if (receivedAdjIteration > currentAdjIteration) {
            if (isApplyingTickAdjustment) {
                LOG_FATAL("Received future adjustment iteration while applying "
                          "the last. current: %u, received: %u",
                          currentAdjIteration, receivedAdjIteration);
            }
            else {
                LOG_FATAL("Out of sequence adjustment iteration. current: %u, "
                          "received: %u",
                          currentAdjIteration, receivedAdjIteration);
            }
        }
    }
}

void WorldClientEndpoint::logNetworkStatistics()
{
    // Dump the stats from the tracker.
    NetStatsDump netStats{NetworkStats::dumpStats()};

    // Log the stats.
    float bytesSentPerSecond{netStats.bytesSent
                             / static_cast<float>(SECONDS_TILL_STATS_DUMP)};
    float bytesReceivedPerSecond{netStats.bytesReceived
                                 / static_cast<float>(SECONDS_TILL_STATS_DUMP)};
    LOG_INFO("Bytes sent per second: %.0f, Bytes received per second: %.0f",
             bytesSentPerSecond, bytesReceivedPerSecond);
}

} // namespace Client
} // namespace AM
