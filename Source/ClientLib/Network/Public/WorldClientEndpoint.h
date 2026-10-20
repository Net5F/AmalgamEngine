#pragma once

#include "SharedConfig.h"
#include "NetworkDefs.h"
#include "MessageProcessor.h"
#include "WorldConnection.h"
#include "WorldMessageFramer.h"
#include "AccountDefs.h"
#include "Log.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/post.hpp"
#include "asio/ssl/context.hpp"
#include "asio/steady_timer.hpp"
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <memory>
#include <atomic>
#include <utility>

namespace AM
{
class EventDispatcher;

namespace Client
{
struct MessageProcessorContext;
class IMessageProcessorExtension;

/**
 * Provides a convenient interface for connecting to the server, sending
 * and receiving messages, and other network-related functionality.
 *
 * Once connected, this connection stays open as long as messages are flowing.
 * When no messages need to be sent, a heartbeat is instead sent from client ->
 * server. If the server receives no messages from a client for a period of
 * time, the connection times out and must be re-established. Similarly, if we
 * receive nothing from the server for a period of time, we disconnect.
 *
 * Note: ServerConnectionSystem is responsible for calling connect() and
 *       disconnect(). If a connection error is detected, this class will push a
 *       ConnectionError event and ServerConnectionSystem will handle it.
 */
class WorldClientEndpoint
{
public:
    WorldClientEndpoint(
        asio::io_context& inIoContext, asio::ssl::context& inSSLContext,
        const MessageProcessorContext& inMessageProcessorContext);

    ~WorldClientEndpoint();

    /**
     * Starts an async connection attempt. Once connected, sends a
     * ConnectionRequest with the given ticket.
     *
     * Note: ServerConnectionSystem is responsible for calling this. See class
     *       comment.
     */
    void connect(const std::array<Uint8, SERVICE_TICKET_BYTES>& worldTicket);

    /**
     * Closes our server connection.
     *
     * Blocks until the IO thread has closed the connection. Once this
     * returns, no more events will be pushed for the closed connection.
     *
     * Note: ServerConnectionSystem is responsible for calling this. See class
     *       comment.
     */
    void disconnect();

    /**
     * If the tick timestep has passed and no messages have been sent since
     * the last heartbeat, sends a heartbeat.
     *
     * Also logs network statistics periodically.
     */
    void tick();

    /**
     * Queues a message to be sent on the IO thread.
     * If we aren't connected, the message is dropped.
     *
     * @param message A structure that defines MESSAGE_TYPE and has an
     * associated serialize() function.
     */
    template<typename Message>
    void send(const Message& message);

    /**
     * Returns the latest tick that we've received an update message for.
     */
    Uint32 getLastReceivedTick();

    // TODO: Just return the total adjustment and let the sim figure it out.
    /**
     * Returns the amount that the sim tick should be adjusted by.
     *
     * The server adds to our tickAdjustment when we're too far ahead or behind
     * it.
     *
     * @return 1 if there's a negative tickAdjustment (the sim can only freeze 1
     *         tick at a time), else 0 or a negative amount equal to the current
     *         tickAdjustment.
     */
    int transferTickAdjustment();

    /**
     * Used for passing us a pointer to the Simulation's currentTick.
     */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Enables the periodic printing and logging of network stats.
     */
    void setNetstatsLoggingEnabled(bool inNetstatsLoggingEnabled);

    /**
     * See MessageProcessor::extension member comment.
     */
    void setMessageProcessorExtension(IMessageProcessorExtension* extension);

private:
    // Note: If a project ever cares to configure any of these, they can be
    //       moved into Config.h.
    static constexpr double CONNECTING_TIMEOUT_S{5};
    /** If we receive nothing for this long, we assume the server is gone.
        Once the sim is running, the server sends us a batch every tick. */
    static constexpr double IDLE_TIMEOUT_S{10};
    /** Large enough to hold a couple max-size messages (which are only sent
        when a dev is modifying the world). */
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{
        2 * CLIENT_MAX_MESSAGE_SIZE};
    /** Large enough to handle bursts, e.g. requesting item data for a full
        inventory. */
    static constexpr std::size_t MAX_QUEUED_WRITES{1000};

    enum class ConnectionState {
        Disconnected,
        /** Establishing the TCP connection and TLS handshake. */
        Connecting,
        /** Connected and ConnectionRequest sent. ServerConnectionSystem
            handles the server's ConnectionResponse. */
        Connected
    };

    /** Connection logic */
    void connectOnIOThread(
        const std::array<Uint8, SERVICE_TICKET_BYTES>& worldTicket);
    void closeOnIOThread();
    void handleConnectionFailure(Uint32 connectionID,
                                 const asio::error_code& error);
    void cancelTimer(asio::steady_timer& timer);

    /** Event handlers. */
    void onSocketConnected(std::shared_ptr<asio::ip::tcp::socket> socket,
                           Uint32 connectionID, const asio::error_code& error);
    void onConnectionReady(Uint32 connectionID);
    void onConnectionDisconnected(Uint32 connectionID,
                                  const asio::error_code& error);
    void onBatchReceived(const WorldBatchComposer::ReceivedBatch& batch);

    /**
     * Queues a framed message on the current connection.
     */
    void sendOnIOThread(BinaryBufferSharedPtr message);

    /**
     * If we haven't sent any messages since the last network tick, sends a
     * heartbeat.
     */
    void sendHeartbeatIfNecessary();

    /**
     * Checks if we need to process the received adjustment, does so if
     * necessary.
     * @param receivedTickAdj The received tick adjustment.
     * @param receivedAdjIteration The adjustment iteration for the received
     *                             adjustment.
     */
    void adjustIfNeeded(Sint8 receivedTickAdj, Uint8 receivedAdjIteration);

    /**
     * Logs the network stats such as bytes sent/received per second.
     */
    void logNetworkStatistics();

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;

    /** Used to dispatch events from the network to the simulation. */
    EventDispatcher& networkEventDispatcher;

    /** Deserializes messages, does any network-layer message handling, and
        pushes messages into the eventDispatcher. */
    MessageProcessor messageProcessor;

    //-------------------------------------------------------------------------
    // IO thread state
    //-------------------------------------------------------------------------
    /** Bounds the TCP connection and TLS handshake. */
    asio::steady_timer connectionTimer;
    std::shared_ptr<asio::ip::tcp::socket> connectingSocket;
    std::shared_ptr<WorldConnection> connection;

    /** The ticket to present once connected. Cleared after it's sent. */
    std::array<Uint8, SERVICE_TICKET_BYTES> pendingWorldTicket;

    ConnectionState connectionState;

    /** The ID of the connection that the IO thread is currently working on.
        Used to discard late async callbacks for old connections. */
    Uint32 currentConnectionID;

    //-------------------------------------------------------------------------
    // Main thread state
    //-------------------------------------------------------------------------
    /** True between connect() and disconnect(). */
    bool connectionActive;

    /** Tracks if we've sent a message since the last network tick.
        Used to determine if we need to heartbeat. */
    unsigned int messagesSentSinceTick;

    /** Pointer to the game's current tick. */
    const std::atomic<Uint32>* currentTickPtr;

    /** The number of seconds we'll wait before logging our network
        statistics. */
    static constexpr unsigned int SECONDS_TILL_STATS_DUMP{5};
    static constexpr unsigned int TICKS_TILL_STATS_DUMP{
        static_cast<unsigned int>(
            (1 / SharedConfig::CLIENT_NETWORK_TICK_TIMESTEP_S)
            * SECONDS_TILL_STATS_DUMP)};

    /** Whether network statistics logging is enabled or not. */
    bool netstatsLoggingEnabled;

    /** The number of ticks since we last logged our network statistics. */
    unsigned int ticksSinceNetstatsLog;

    //-------------------------------------------------------------------------
    // Shared state
    //-------------------------------------------------------------------------
    /** The adjustment that the server has told us to apply to the tick. */
    std::atomic<int> tickAdjustment;

    /** Tracks what iteration of tick offset adjustments we're on.
        Used to make sure that we don't double-count adjustments. */
    std::atomic<Uint8> adjustmentIteration;

    /** True when we're waiting for the sim to finish applying an adjustment. */
    std::atomic<bool> isApplyingTickAdjustment;
};

template<typename Message>
void WorldClientEndpoint::send(const Message& message)
{
    if (!connectionActive) {
        LOG_INFO("Tried to send while server is disconnected.");
        return;
    }

    // Frame the message here, so the IO thread only has to queue it.
    BinaryBufferSharedPtr framedMessage{
        WorldMessageFramer::frameMessage(message, adjustmentIteration)};
    if (!framedMessage) {
        return;
    }

    messagesSentSinceTick++;
    asio::post(ioContext,
               [this, framedMessage = std::move(framedMessage)]() mutable {
                   sendOnIOThread(std::move(framedMessage));
               });
}

} // namespace Client
} // namespace AM
