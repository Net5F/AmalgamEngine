#include "WorldClientEndpoint.h"
#include "MessageProcessorContext.h"
#include "ClientConnectionEvent.h"
#include "ConnectionRequest.h"
#include "Shared/AccountServiceEndpoint.h"
#include "IMessageProcessorExtension.h"
#include "Deserialize.h"
#include "Config.h"
#include "UserConfig.h"
#include "NetworkStats.h"
#include "Log.h"
#include "asio/error.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/post.hpp"
#include "tracy/Tracy.hpp"
#include <openssl/crypto.h>
#include <functional>
#include <utility>

namespace AM
{
namespace WorldServer
{

WorldClientEndpoint::WorldClientEndpoint(
    asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
    const MessageProcessorContext& inMessageProcessorContext,
    AccountServiceEndpoint& inAccountEndpoint)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, accountEndpoint{inAccountEndpoint}
, eventDispatcher{}
, messageProcessor{inMessageProcessorContext}
, maxClients{UserConfig::get().getMaxClients()}
, clientRegistry(maxClients + NETWORK_ID_REUSE_BUFFER)
, acceptor(inIoContext,
           asio::ip::tcp::endpoint{asio::ip::tcp::v4(), Config::SERVER_PORT},
           std::bind_front(&WorldClientEndpoint::connectionFactory, this),
           std::bind_front(&WorldClientEndpoint::onConnectionAccepted, this),
           std::bind_front(&WorldClientEndpoint::onAcceptorError, this))
, batchFramer{}
, outbox{}
, ticksSinceNetstatsLog{0}
, currentTickPtr{nullptr}
{
}

void WorldClientEndpoint::start()
{
    asio::post(ioContext, [this]() {
        if (!(acceptor.start())) {
            LOG_FATAL("Failed to start listening for clients on port %u.",
                      Config::SERVER_PORT);
        }
    });
}

void WorldClientEndpoint::tick()
{
    ZoneScoped;

    // Hand everything that the sim queued to the IO thread.
    // Note: We post even if the outbox is empty, since clients may still
    //       need explicit confirmations or tick adjustments.
    Uint32 currentTick{getCurrentTick()};
    std::size_t outboxSize{outbox.size()};
    asio::post(ioContext,
               [this, entries = std::move(outbox), currentTick]() mutable {
                   flushOnIOThread(entries, currentTick);
               });
    outbox = {};
    outbox.reserve(outboxSize);

    // If it's time to log our network statistics, do so.
    ticksSinceNetstatsLog++;
    if (ticksSinceNetstatsLog == TICKS_TILL_STATS_DUMP) {
        logNetworkStatistics();
        ticksSinceNetstatsLog = 0;
    }
}

void WorldClientEndpoint::sendBytes(NetworkID networkID,
                                    const BinaryBufferSharedPtr& message,
                                    Uint32 messageTick)
{
    if (!networkID || !message) {
        return;
    }

    outbox.emplace_back(OutboxEntry{.netID{networkID},
                                    .message{message},
                                    .messageTick{messageTick}});
}

void WorldClientEndpoint::disconnect(NetworkID networkID,
                                     DisconnectNotice::Reason reason)
{
    if (!networkID) {
        return;
    }

    outbox.emplace_back(
        OutboxEntry{.netID{networkID}, .disconnectReason{reason}});
}

void WorldClientEndpoint::disconnectAll(DisconnectNotice::Reason reason)
{
    outbox.emplace_back(
        OutboxEntry{.netID{NULL_NETWORK_ID}, .disconnectReason{reason}});
}

EventDispatcher& WorldClientEndpoint::getEventDispatcher()
{
    return eventDispatcher;
}

void WorldClientEndpoint::registerCurrentTickPtr(
    const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void WorldClientEndpoint::setMessageProcessorExtension(
    IMessageProcessorExtension* extension)
{
    messageProcessor.setExtension(extension);
}

std::shared_ptr<WorldClientConnection>
    WorldClientEndpoint::connectionFactory(asio::ip::tcp::socket socket)
{
    // Disable Nagle's algorithm. Most of our messages are small and
    // latency-sensitive.
    asio::error_code error{};
    socket.set_option(asio::ip::tcp::no_delay{true}, error);
    if (error) {
        LOG_INFO("Failed to configure client socket: %s",
                 error.message().c_str());
        return {};
    }

    // Configure and create a new connection.
    // Note: Until a client authenticates, the only message it can send is
    //       its ConnectionRequest, so we limit the payload size to match.
    ConnectionRequest connectionRequest{};
    TlsTransport::ClassConfig tlsTransportConfig{
        .handshakeTimeoutS{HANDSHAKE_TIMEOUT_S},
        .maxQueuedWriteBytes{MAX_QUEUED_WRITE_BYTES},
        .maxQueuedMessages{MAX_QUEUED_WRITES}};
    WorldClientConnection::ClassConfig connectionConfig{
        .tlsTransportConfig{tlsTransportConfig},
        .idleTimeoutS{Config::CLIENT_TIMEOUT_S},
        .maxReadPayloadSize{Serialize::measureSize(connectionRequest)}};

    return std::make_shared<WorldClientConnection>(
        std::move(socket), sslContext, connectionConfig);
}

void WorldClientEndpoint::onConnectionAccepted(
    std::shared_ptr<WorldClientConnection> connection)
{
    if (clientRegistry.size() >= maxClients) {
        LOG_INFO("Rejected connection attempt: Already at maximum connected "
                 "clients.");
        connection->close();
        return;
    }

    // Try to add the client to the registry.
    std::shared_ptr<Client> client{std::make_shared<Client>(connection)};
    std::optional<ConnectionHandle> handle{clientRegistry.add(client)};
    if (!handle) {
        connection->close();
        return;
    }
    client->setHandle(*handle);
    LOG_INFO("New client connected. Assigning netID: %u", client->getNetID());

    // Successfully added. Register callbacks and start the handshake process.
    // Note: We don't notify the sim about this client until it authenticates
    //       (see onTicketResult()).
    connection->setMessageCallback(
        [this, handle = *handle](
            const WorldClientMessageComposer::ReceivedMessage& message) {
            onMessageReceived(handle, message);
        });
    connection->startAuthTimeout(AUTH_TIMEOUT_S);
    connection->startServer(
        [](AM::Connection&) {
            // Don't need to do anything on connection ready.
        },
        [this, handle = *handle](const asio::error_code& error) {
            onConnectionDisconnected(handle, error);
        });
}

void WorldClientEndpoint::onAcceptorError(const asio::error_code& error)
{
    LOG_INFO("World client acceptor error: %s", error.message().c_str());
}

void WorldClientEndpoint::onConnectionDisconnected(
    ConnectionHandle handle, const asio::error_code& error)
{
    std::shared_ptr<Client> client{clientRegistry.find(handle)};
    if (!client) {
        return;
    }

    // If we didn't initiate this disconnect, the connection was lost.
    NetworkID netID{client->getNetID()};
    if (!(client->isDisconnecting())) {
        // Note: Timeouts are common, and the OS's message for them is long,
        //       so we print a short message instead.
        if (error == asio::error::timed_out) {
            LOG_INFO("Client disconnected: Timeout. NetID: %u", netID);
        }
        else {
            LOG_INFO("Client disconnected: %s NetID: %u",
                     error.message().c_str(), netID);
        }

        // If the sim knows about this client, tell it.
        if (client->getAuthState() == Client::AuthState::Authenticated) {
            eventDispatcher.emplace<ClientConnectionEvent>(
                ClientDisconnected{.clientID{netID}});
        }
    }

    clientRegistry.erase(handle);
}

void WorldClientEndpoint::onMessageReceived(
    ConnectionHandle handle,
    const WorldClientMessageComposer::ReceivedMessage& message)
{
    std::shared_ptr<Client> client{clientRegistry.find(handle)};
    if (!client || client->isDisconnecting()) {
        return;
    }

    NetworkStats::recordBytesReceived(CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE
                                      + message.payload.size());

    if (!(client->processAdjustmentIteration(message.adjustmentIteration))) {
        LOG_INFO("Dropped client: Invalid adjustment iteration. NetID: %u",
                 client->getNetID());
        client->getConnection().disconnect(
            asio::error::make_error_code(asio::error::invalid_argument));
        return;
    }

    // Until a client authenticates, the only message we accept from it is
    // its ConnectionRequest.
    if (message.type
        == static_cast<Uint8>(EngineMessageType::ConnectionRequest)) {
        processConnectionRequest(*client, message.payload);
    }
    else if (client->getAuthState() == Client::AuthState::Authenticated) {
        processReceivedMessage(*client, message.type, message.payload);
    }
}

void WorldClientEndpoint::processConnectionRequest(Client& client,
                                                   std::span<Uint8> payload)
{
    NetworkID netID{client.getNetID()};
    if (client.getAuthState() != Client::AuthState::AwaitingRequest) {
        LOG_INFO("Ignoring unexpected ConnectionRequest. NetID: %u", netID);
        return;
    }

    ConnectionRequest connectionRequest{};
    bool deserialized{Deserialize::fromBuffer(payload.data(), payload.size(),
                                              connectionRequest)};
    OPENSSL_cleanse(payload.data(), payload.size());
    if (!deserialized) {
        LOG_INFO("Rejected client: Failed to deserialize ConnectionRequest. "
                 "NetID: %u",
                 netID);
        rejectClient(client, ConnectionResponse::InvalidTicket);
        return;
    }

    // Ask AccountServer to validate the ticket.
    client.setAuthState(Client::AuthState::Validating);
    accountEndpoint.consumeServiceTicket(
        connectionRequest.ticket,
        [this, handle = client.getHandle()](
            const ConsumeServiceTicketResponse& response) {
            onTicketResult(handle, response);
        });
    OPENSSL_cleanse(connectionRequest.ticket.data(),
                    connectionRequest.ticket.size());
}

void WorldClientEndpoint::onTicketResult(
    ConnectionHandle handle, const ConsumeServiceTicketResponse& response)
{
    // If the client disconnected while we were waiting, there's nothing to
    // do.
    std::shared_ptr<Client> client{clientRegistry.find(handle)};
    if (!client || client->isDisconnecting()) {
        return;
    }

    NetworkID netID{client->getNetID()};
    if (client->getAuthState() != Client::AuthState::Validating) {
        LOG_ERROR("Received ticket result for client that isn't validating. "
                  "NetID: %u",
                  netID);
        return;
    }

    switch (response.result) {
        case ConsumeServiceTicketResponse::Success: {
            // If this account is already logged in, disconnect the old
            // client.
            // Note: The old client's disconnect event must reach the sim
            //       before this client's connect event, so the sim can hand
            //       the account's entity over to this client.
            disconnectAccountClients(response.accountID);

            client->setAuthState(Client::AuthState::Authenticated);
            client->setAccountID(response.accountID);
            client->getConnection().cancelAuthTimeout();
            client->getConnection().setMaxReadPayloadSize(
                CLIENT_MAX_MESSAGE_SIZE - CLIENT_HEADER_SIZE
                - MESSAGE_HEADER_SIZE);
            LOG_INFO("Client authenticated. NetID: %u, AccountID: %lld", netID,
                     static_cast<long long>(response.accountID));

            // Notify the sim that a client was connected.
            eventDispatcher.emplace<ClientConnectionEvent>(
                ClientConnected{netID, response.accountID, response.username});
            break;
        }
        case ConsumeServiceTicketResponse::InvalidTicket: {
            LOG_INFO("Rejected client: Invalid ticket. NetID: %u", netID);
            rejectClient(*client, ConnectionResponse::InvalidTicket);
            break;
        }
        default: {
            LOG_INFO("Rejected client: Failed to validate ticket. NetID: %u",
                     netID);
            rejectClient(*client, ConnectionResponse::InternalError);
            break;
        }
    }
}

void WorldClientEndpoint::rejectClient(Client& client,
                                       ConnectionResponse::Result result)
{
    client.setAuthState(Client::AuthState::Rejected);

    batchFramer.begin();
    batchFramer.appendMessage(ConnectionResponse{.result{result}});
    client.getConnection().sendFramed(batchFramer.finish(0, 0));

    beginClosingClient(client, std::nullopt);
}

void WorldClientEndpoint::disconnectAccountClients(Sint64 accountID)
{
    clientRegistry.forEach(
        [&](ConnectionHandle, const std::shared_ptr<Client>& client) {
            if ((client->getAuthState() == Client::AuthState::Authenticated)
                && !(client->isDisconnecting())
                && (client->getAccountID() == accountID)) {
                LOG_INFO("Disconnecting client: Account logged in from "
                         "another connection. NetID: %u, AccountID: %lld",
                         client->getNetID(), static_cast<long long>(accountID));
                disconnectClient(*client, DisconnectNotice::LoggedInElsewhere);
            }
        });
}

void WorldClientEndpoint::processReceivedMessage(Client& client,
                                                 Uint8 messageType,
                                                 std::span<Uint8> payload)
{
    // Process the message.
    // Note: messageTick will be > -1 if the message contained a tick number.
    Sint64 messageTick{messageProcessor.processReceivedMessage(
        client.getNetID(), messageType, payload.data(), payload.size())};

    // If the message carried a tick number, use it to calc a diff and give it
    // to the client.
    if (messageTick != -1) {
        Sint64 tickDiff{messageTick - static_cast<Sint64>(getCurrentTick())};
        if (!(client.recordTickDiff(tickDiff))) {
            LOG_INFO("Dropped client: tickDiff out of range. tickDiff: %lld, "
                     "NetID: %u",
                     static_cast<long long>(tickDiff), client.getNetID());
            client.getConnection().disconnect(
                asio::error::make_error_code(asio::error::invalid_argument));
        }
    }
}

void WorldClientEndpoint::disconnectClient(Client& client,
                                           DisconnectNotice::Reason reason)
{
    client.queueDisconnect(reason);
    NetworkStats::recordBytesSent(
        client.sendQueuedMessages(batchFramer, getCurrentTick()));
    beginClosingClient(client, reason);
}

void WorldClientEndpoint::beginClosingClient(
    Client& client, std::optional<DisconnectNotice::Reason> reason)
{
    // If we're already closing, or a send failure already disconnected the
    // client, there's nothing to do.
    if (client.isDisconnecting() || !(client.getConnection().isReady())) {
        return;
    }

    client.setDisconnecting();
    client.getConnection().closeAfterWrites(LINGER_TIMEOUT_S);

    // If the sim knows about this client, tell it.
    if (client.getAuthState() == Client::AuthState::Authenticated) {
        eventDispatcher.emplace<ClientConnectionEvent>(
            ClientDisconnected{.clientID{client.getNetID()}, .reason{reason}});
    }
}

void WorldClientEndpoint::flushOnIOThread(std::vector<OutboxEntry>& entries,
                                          Uint32 currentTick)
{
    ZoneScoped;

    // Give each client the messages and disconnects that were queued for it.
    for (OutboxEntry& entry : entries) {
        // If this is a disconnectAll(), apply it to every client.
        if (!entry.message && !entry.netID) {
            clientRegistry.forEach([&](ConnectionHandle,
                                       const std::shared_ptr<Client>& client) {
                if (client->isDisconnecting()) {
                    return;
                }
                if (client->getAuthState()
                    == Client::AuthState::Authenticated) {
                    client->queueDisconnect(entry.disconnectReason);
                }
                else {
                    client->getConnection().disconnect(
                        asio::error::make_error_code(asio::error::shut_down));
                }
            });

            if (entry.disconnectReason == DisconnectNotice::ServerShutdown) {
                acceptor.stop();
            }
            continue;
        }

        // Note: The sim only knows about authenticated clients.
        std::shared_ptr<Client> client{findClient(entry.netID)};
        if (!client
            || (client->getAuthState() != Client::AuthState::Authenticated)
            || client->isDisconnecting()) {
            continue;
        }

        if (entry.message) {
            client->queueMessage(std::move(entry.message), entry.messageTick);
        }
        else {
            client->queueDisconnect(entry.disconnectReason);
        }
    }

    // Send each client's batch, and begin any requested disconnects.
    std::size_t bytesSent{0};
    clientRegistry.forEach(
        [&](ConnectionHandle, const std::shared_ptr<Client>& client) {
            if ((client->getAuthState() != Client::AuthState::Authenticated)
                || client->isDisconnecting()) {
                return;
            }

            bytesSent += client->sendQueuedMessages(batchFramer, currentTick);
            if (std::optional<DisconnectNotice::Reason> reason{
                    client->getQueuedDisconnect()}) {
                beginClosingClient(*client, reason);
            }
        });

    NetworkStats::recordBytesSent(bytesSent);
}

std::shared_ptr<Client> WorldClientEndpoint::findClient(NetworkID networkID)
{
    return clientRegistry.find(clientRegistry.getHandle(networkID));
}

Uint32 WorldClientEndpoint::getCurrentTick()
{
    // Note: This should only ever be nullptr if the main thread crashes during
    //       startup.
    if (currentTickPtr) {
        return *currentTickPtr;
    }
    else {
        return 0;
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

} // namespace WorldServer
} // namespace AM
