#pragma once

#include "SharedConfig.h"
#include "NetworkDefs.h"
#include "ClientNetworkDefs.h"
#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Serialize.h"
#include "Peer.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "Timer.h"
#include "Log.h"
#include <SDL_stdinc.h>
#include <string>
#include <memory>
#include <atomic>
#include <thread>

namespace AM
{
struct ConnectionResponse;
struct EntityUpdate;

namespace Client
{
class IMessageProcessorExtension;
/**
 * Provides a convenient interface for connecting to the server, sending
 * and receiving messages, and other network-related functionality.
 *
 * Note: ServerConnectionSystem is responsible for calling connect() and
 * disconnect(). If a connection error is detected, this class will push a
 * ConnectionError event and ServerConnectionSystem will handle it.
 */
class Network
{
public:
    Network();

    ~Network();

    // TODO: If the IP provided in UserConfig.json is invalid, the receive
    //       thread will stall for a long time while waiting for
    //       SDLNet_TCP_Open() to return. If we move off SDL_Net, improve this.
    /**
     * Spins up the connectAndReceive thread.
     *
     * Note: ServerConnectionSystem is responsible for calling this. See class
     *       comment.
     */
    void connect();

    /**
     * Cleans up our server connection and spins down the receive thread.
     *
     * Note: ServerConnectionSystem is responsible for calling this. See class
     *       comment.
     */
    void disconnect();

    /**
     * Updates accumulatedTime. If greater than the tick timestep and no
     * messages have been sent since the last heartbeat, sends a message.
     *
     * Also logs network statistics periodically.
     */
    void tick();

    /**
     * Sends bytes over the network.
     * Errors if the server is disconnected.
     *
     * @param messageStruct  A structure that defines MESSAGE_TYPE and has an
     *                       associated serialize() function.
     */
    template<typename T>
    void serializeAndSend(const T& messageStruct);

    /**
     * Returns the Network event dispatcher. All messages that we receive
     * from the server are pushed into this dispatcher.
     */
    EventDispatcher& getEventDispatcher();

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
     * Used for passing us a pointer to the Game's currentTick.
     */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Enables the periodic printing and logging of network stats.
     */
    void setNetstatsLoggingEnabled(bool inNetstatsLoggingEnabled);

    /**
     * See MessageProcessor::extension member comment.
     */
    void setMessageProcessorExtension(
        std::unique_ptr<IMessageProcessorExtension> extension);

private:
    /**
     * How long the receive loop in connectAndReceive should delay if no
     * socket activity was reported on the socket.
     */
    static constexpr unsigned int INACTIVE_DELAY_TIME_MS{1};

    /**
     * Sends bytes over the network.
     * Errors if the server is disconnected.
     */
    void send(const BinaryBufferSharedPtr& message);

    /**
     * If we haven't sent any messages since the last network tick, sends a
     * heartbeat.
     */
    void sendHeartbeatIfNecessary();

    /**
     * Thread function, started from connect().
     *
     * First, tries to connect to the server.
     * If successful, receives messages from the server and passes them to
     * processBatch().
     */
    void connectAndReceive();

    /**
     * Processes the received header and following batch.
     * If any messages are expected, receives the messages.
     */
    void processBatch();

    /**
     * Checks if we need to process the received adjustment, does so if
     * necessary.
     * @param receivedTickAdj  The received tick adjustment.
     * @param receivedAdjIteration  The adjustment iteration for the received
     *                              adjustment.
     */
    void adjustIfNeeded(Sint8 receivedTickAdj, Uint8 receivedAdjIteration);

    /**
     * Logs the network stats such as bytes sent/received per second.
     */
    void logNetworkStatistics();

    std::shared_ptr<Peer> server;

    /** Used to dispatch events from the network to the simulation. */
    EventDispatcher eventDispatcher;

    /** Deserializes messages, does any network-layer message handling, and
        pushes messages into the eventDispatcher. */
    MessageProcessor messageProcessor;

    /** The adjustment that the server has told us to apply to the tick. */
    std::atomic<int> tickAdjustment;

    /** Tracks what iteration of tick offset adjustments we're on.
        Used to make sure that we don't double-count adjustments. */
    std::atomic<Uint8> adjustmentIteration;

    /** True when we're waiting for the sim to finish applying an adjustment. */
    std::atomic<bool> isApplyingTickAdjustment;

    /** Tracks if we've sent a message since the last network tick.
        Used to determine if we need to heartbeat. */
    unsigned int messagesSentSinceTick;

    /** Pointer to the game's current tick. */
    const std::atomic<Uint32>* currentTickPtr;

    /** Calls pollForMessages(). */
    std::jthread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;

    /** Holds a received server header while we process it. */
    BinaryBuffer headerRecBuffer;
    /** Holds a received message batch while we pass its messages to
        MessageProcessor. */
    BinaryBuffer batchRecBuffer;
    /** If a batch is compressed, it's decompressed into this buffer before
        processing. */
    BinaryBuffer decompressedBatchRecBuffer;

    /** The number of seconds we'll wait before logging our network
        statistics. */
    static constexpr unsigned int SECONDS_TILL_STATS_DUMP{5};
    static constexpr unsigned int TICKS_TILL_STATS_DUMP{
        static_cast<unsigned int>((1 / SharedConfig::NETWORK_TICK_TIMESTEP_S)
                                  * SECONDS_TILL_STATS_DUMP)};

    /** Whether network statistics logging is enabled or not. */
    bool netstatsLoggingEnabled;

    /** The number of ticks since we last logged our network statistics. */
    unsigned int ticksSinceNetstatsLog;
};

template<typename T>
void Network::serializeAndSend(const T& messageStruct)
{
    // Allocate the buffer.
    std::size_t totalMessageSize{CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE
                                 + Serialize::measureSize(messageStruct)};
    BinaryBufferSharedPtr messageBuffer{
        std::make_shared<BinaryBuffer>(totalMessageSize)};

    // Serialize the message struct into the buffer, leaving room for the
    // headers.
    std::size_t messageSize{Serialize::toBuffer(
        messageBuffer->data(), messageBuffer->size(), messageStruct,
        (CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE))};

    // Check that the message isn't too big.
    // Note: We don't compress messages on this side, so we know the final
    //       message size at this point.
    if ((totalMessageSize > Peer::MAX_WIRE_SIZE)
        || (messageSize > UINT16_MAX)) {
        LOG_FATAL("Tried to send a too-large message. Size: %u, max: %u",
                  totalMessageSize, Peer::MAX_WIRE_SIZE);
    }

    // Copy the adjustment iteration into the client header.
    messageBuffer->at(ClientHeaderIndex::AdjustmentIteration)
        = adjustmentIteration;

    // Copy the message type into the message header.
    // TODO: Add a nice compile-time message if T doesn't define MESSAGE_TYPE.
    messageBuffer->at(CLIENT_HEADER_SIZE + MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(T::MESSAGE_TYPE);

    // Copy the message size into the message header.
    ByteTools::write16(static_cast<Uint16>(messageSize),
                       (messageBuffer->data() + CLIENT_HEADER_SIZE
                        + MessageHeaderIndex::Size));

    // Send the message.
    send(messageBuffer);
}

} // namespace Client
} // namespace AM
