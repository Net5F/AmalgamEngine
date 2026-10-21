#pragma once

#include "SharedConfig.h"
#include "NetworkID.h"
#include "NetworkDefs.h"
#include "Client.h"
#include "WorldClientConnection.h"
#include "WorldBatchFramer.h"
#include "MessageProcessor.h"
#include "ConnectionRegistry.h"
#include "Acceptor.h"
#include "ConnectionResponse.h"
#include "ConsumeServiceTicketResponse.h"
#include "DisconnectNotice.h"
#include "Serialize.h"
#include "ByteTools.h"
#include "QueuedEvents.h"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include <atomic>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace AM
{
class AccountServiceEndpoint;

namespace WorldServer
{
struct MessageProcessorContext;
class IMessageProcessorExtension;

/**
 * Manages communication with clients (ran by users).
 *
 * Provides a convenient interface for sending and receiving messages, and
 * other network-related functionality.
 *
 * Newly accepted clients must send a ConnectionRequest containing a World
 * Server ticket. We validate the ticket with AccountServer, and only tell
 * the sim about the client once it succeeds. Until then, any other messages
 * from the client are dropped.
 *
 * Once connected, this connection stays open as long as messages are flowing.
 * When no messages need to be sent, a heartbeat is instead sent from client ->
 * server. If the server receives no messages from a client for a period of
 * time, the connection times out and must be re-established.
 *
 * Messages that the sim sends are held until the next tick(), then handed to
 * the network IO thread all at once.
 *
 * Note: The public functions (other than start()) must be called from the
 *       main thread. All other work happens on the network IO thread.
 */
class WorldClientEndpoint
{
public:
    /**
     * @param inSSLContext Must present our certificate.
     */
    WorldClientEndpoint(
        asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
        const MessageProcessorContext& inMessageProcessorContext,
        AccountServiceEndpoint& inAccountEndpoint);

    /**
     * Starts accepting clients.
     */
    void start();

    /**
     * Hands all messages that were sent since the last tick to the network
     * IO thread, which sends them.
     *
     * Also logs network statistics periodically.
     */
    void tick();

    /**
     * Queues a message to be sent on the next tick().
     * Equivalent to calling serialize() and sendBytes().
     *
     * @param networkID The client to send the message to.
     * @param message A structure that defines MESSAGE_TYPE and has an
     * associated serialize() function.
     * @param messageTick Optional, used in certain cases to update the
     * Client's latestSentSimTick.
     */
    template<typename Message>
    void send(NetworkID networkID, const Message& message,
              Uint32 messageTick = 0);

    /**
     * Serializes and frames the given message.
     *
     * @param message A structure that defines MESSAGE_TYPE and has an
     *                associated serialize() function.
     * @return A message that's ready to be passed to sendBytes().
     */
    template<typename Message>
    BinaryBufferSharedPtr serialize(const Message& message);

    /**
     * Queues a serialized message to be sent on the next tick().
     * If the client has disconnected, the message is dropped.
     *
     * @param networkID The client to send the message to.
     * @param message The message to send.
     * @param messageTick Optional, used when sending entity movement updates
     * to update the Client's latestSentSimTick.
     */
    void sendBytes(NetworkID networkID, const BinaryBufferSharedPtr& message,
                   Uint32 messageTick = 0);

    /**
     * Disconnects the given client on the next tick(), after sending it any
     * messages that were queued before this call, followed by a
     * DisconnectNotice with the given reason.
     *
     * A ClientDisconnected event with the given reason will be pushed once
     * the disconnect begins. Messages sent to the client after this call are
     * dropped.
     */
    void disconnect(NetworkID networkID, DisconnectNotice::Reason reason);

    /**
     * Disconnects all clients, as if disconnect() was called for each.
     * Clients that haven't finished authenticating are dropped.
     *
     * If the reason is ServerShutdown, also stops accepting new clients.
     */
    void disconnectAll(DisconnectNotice::Reason reason);

    /**
     * Returns the endpoint's event dispatcher. All messages that we receive
     * from clients are pushed into this dispatcher.
     */
    EventDispatcher& getEventDispatcher();

    /** Used for passing us a pointer to the sim's currentTick. */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * See MessageProcessor::extension member comment.
     */
    void setMessageProcessorExtension(IMessageProcessorExtension* extension);

private:
    // Note: If a project ever cares to configure any of these, they can be
    //       moved into Config.h.
    /** How long a client has to complete the TLS handshake. */
    static constexpr double HANDSHAKE_TIMEOUT_S{5};
    /** How long a client has to authenticate after connecting, before we
        drop it. */
    static constexpr double AUTH_TIMEOUT_S{10};
    /** When we disconnect a client, how long we'll wait for it to receive
        our final messages and close its side. */
    static constexpr double LINGER_TIMEOUT_S{2};
    /** If a client falls this far behind on receiving our batches, we
        consider it too slow (or hostile) and disconnect it.
        At our network tick rate, this is about 2 seconds of max-size
        batches. */
    static constexpr std::size_t MAX_QUEUED_WRITES{40};
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{
        10 * SharedConfig::MAX_BATCH_SIZE};
    /** Network IDs are registry slot IDs. Since IDs are reserved by marching
        forward, these extra slots make it unlikely that an ID will be reused
        before the sim has processed the disconnect of its previous owner. */
    static constexpr std::size_t NETWORK_ID_REUSE_BUFFER{100};

    /** The number of seconds we'll wait before logging our network
        statistics. */
    static constexpr unsigned int SECONDS_TILL_STATS_DUMP{5};
    static constexpr unsigned int TICKS_TILL_STATS_DUMP{
        static_cast<unsigned int>(
            (1 / SharedConfig::SERVER_NETWORK_TICK_TIMESTEP_S)
            * SECONDS_TILL_STATS_DUMP)};

    /** A message or disconnect that the sim queued for a client. */
    struct OutboxEntry {
        /** The client that this entry is for. If null and disconnectReason
            is set, applies to all clients. */
        NetworkID netID{NULL_NETWORK_ID};
        /** The message to send. If null, this is a disconnect. */
        BinaryBufferSharedPtr message{};
        /** See sendBytes(). */
        Uint32 messageTick{0};
        /** If message is null, the reason to disconnect with. */
        DisconnectNotice::Reason disconnectReason{};
    };

    // Acceptor callbacks
    std::shared_ptr<WorldClientConnection>
        connectionFactory(asio::ip::tcp::socket socket);
    void
        onConnectionAccepted(std::shared_ptr<WorldClientConnection> connection);
    void onAcceptorError(const asio::error_code& error);

    // Connection callbacks
    void onConnectionDisconnected(ConnectionHandle handle,
                                  const asio::error_code& error);
    void onMessageReceived(
        ConnectionHandle handle,
        const WorldClientMessageComposer::ReceivedMessage& message);

    // Authentication
    void processConnectionRequest(Client& client, std::span<Uint8> payload);
    void onTicketResult(ConnectionHandle handle,
                        const ConsumeServiceTicketResponse& response);

    /**
     * Sends the given client a failed ConnectionResponse, then closes the
     * connection.
     */
    void rejectClient(Client& client, ConnectionResponse::Result result);

    /**
     * Disconnects any authenticated clients that are logged in to the given
     * account.
     */
    void disconnectAccountClients(Sint64 accountID);

    /**
     * Passes a received message to the MessageProcessor. If it contained a
     * tick number, records the client's tick diff.
     */
    void processReceivedMessage(Client& client, Uint8 messageType,
                                std::span<Uint8> payload);

    /**
     * Sends any messages queued for the client (followed by a
     * DisconnectNotice with the given reason), then begins closing its
     * connection.
     */
    void disconnectClient(Client& client, DisconnectNotice::Reason reason);

    /**
     * Begins closing the given client's connection. If the sim knows about
     * the client, pushes a ClientDisconnected event.
     *
     * Note: The client stays in the registry until the connection finishes
     *       closing, so its network ID isn't reused early.
     */
    void beginClosingClient(Client& client,
                            std::optional<DisconnectNotice::Reason> reason);

    /**
     * Sends the given outbox entries to their clients.
     */
    void flushOnIOThread(std::vector<OutboxEntry>& entries, Uint32 currentTick);

    /**
     * Returns the client with the given network ID, or nullptr if there
     * isn't one.
     */
    std::shared_ptr<Client> findClient(NetworkID networkID);

    /**
     * Returns the sim's current tick.
     */
    Uint32 getCurrentTick();

    /**
     * Logs the network stats such as bytes sent/received per second.
     */
    void logNetworkStatistics();

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;

    /** Used to validate the tickets that clients send us. */
    AccountServiceEndpoint& accountEndpoint;

    /** Used to dispatch events from the network to the simulation. */
    EventDispatcher eventDispatcher;

    /** Deserializes messages, does any network-layer message handling, and
        passes messages down to the simulation. */
    MessageProcessor messageProcessor;

    //-------------------------------------------------------------------------
    // IO thread state
    //-------------------------------------------------------------------------
    /** The maximum number of clients that we'll allow at once. */
    std::size_t maxClients;

    ConnectionRegistry<Client> clientRegistry;

    Acceptor<WorldClientConnection> acceptor;

    /** Used to build every client's batches. */
    WorldBatchFramer batchFramer;

    //-------------------------------------------------------------------------
    // Main thread state
    //-------------------------------------------------------------------------
    /** Messages and disconnects that the sim has queued since the last
        tick(). */
    std::vector<OutboxEntry> outbox;

    /** The number of ticks since we last logged our network statistics. */
    unsigned int ticksSinceNetstatsLog;

    /** Pointer to the sim's current tick. */
    const std::atomic<Uint32>* currentTickPtr;
};

template<typename Message>
void WorldClientEndpoint::send(NetworkID networkID, const Message& message,
                               Uint32 messageTick)
{
    // Serialize and frame the message.
    BinaryBufferSharedPtr messageBuffer{serialize(message)};

    // Send the message.
    sendBytes(networkID, messageBuffer, messageTick);
}

template<typename Message>
BinaryBufferSharedPtr WorldClientEndpoint::serialize(const Message& message)
{
    // Allocate the buffer.
    std::size_t totalMessageSize{MESSAGE_HEADER_SIZE
                                 + Serialize::measureSize(message)};
    BinaryBufferSharedPtr messageBuffer{
        std::make_shared<BinaryBuffer>(totalMessageSize)};

    // Serialize the message struct into the buffer, leaving room for the
    // header.
    std::size_t messageSize{Serialize::toBuffer(messageBuffer->data(),
                                                messageBuffer->size(), message,
                                                MESSAGE_HEADER_SIZE)};

    // Copy the type into the buffer.
    // TODO: Add a nice compile-time message if Message doesn't define
    //       MESSAGE_TYPE.
    messageBuffer->at(MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(Message::MESSAGE_TYPE);

    // Copy the messageSize into the buffer.
    ByteTools::write16(static_cast<Uint16>(messageSize),
                       (messageBuffer->data() + MessageHeaderIndex::Size));

    return messageBuffer;
}

} // namespace WorldServer
} // namespace AM
