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
#include <string>
#include <memory>
#include <atomic>
#include <thread>

namespace AM
{
class ConnectionResponse;
class EntityUpdate;

namespace Client
{
/**
 * Provides network functionality in the format that the Simulation wants.
 */
class Network
{
public:
    Network();

    ~Network();

    /**
     * Attempts to establish a connection with the server.
     * If successful, starts the receiver thread.
     *
     * @return true if a connection was successfully established, else false.
     */
    bool connect();

    /**
     * Updates accumulatedTime. If greater than the tick timestep and no
     * messages have been sent since the last heartbeat, sends a message.
     *
     * Also logs network statistics if it's time to do so.
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
     * Subtracts an appropriate amount from the tickAdjustment based on its
     * current value, and returns the amount subtracted.
     * @return 1 if there's a negative tickAdjustment (the sim can only freeze 1
     *         tick at a time), else 0 or a negative amount equal to the current
     *         tickAdjustment.
     */
    int transferTickAdjustment();

    /**
     * Returns the event dispatcher.
     *
     * Used by the simulation's systems to subscribe their event queues.
     */
    EventDispatcher& getDispatcher();

    /**
     * Used for passing us a pointer to the Game's currentTick.
     */
    void registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Enables the periodic printing and logging of network stats.
     */
    void setNetstatsLoggingEnabled(bool inNetstatsLoggingEnabled);

private:
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
     * Tries to retrieve a message batch from the server.
     * If successful, calls processBatch().
     */
    int pollForMessages();

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

    /** Used to send received messages and network events to the subscribed
        systems. */
    EventDispatcher dispatcher;

    /** Deserializes messages, does any network-layer message handling, and
        passes messages down to the simulation. */
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

    /** Tracks how long it's been since we've received a message from the
        server. */
    Timer receiveTimer;

    /** Calls pollForMessages(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;

    /** Holds a received server header while we process it. */
    BinaryBuffer headerRecBuffer;
    /** Holds a received message batch while we pass its messages to
        MessageProcessor. */
    BinaryBuffer batchRecBuffer;
    /** If a batch is compressed, it's uncompressed into this buffer before
        processing. */
    BinaryBuffer uncompressedBatchRecBuffer;

    /** The number of seconds we'll wait before logging our network
        statistics. */
    static constexpr unsigned int SECONDS_TILL_STATS_DUMP = 5;
    static constexpr unsigned int TICKS_TILL_STATS_DUMP
        = (1 / SharedConfig::NETWORK_TICK_TIMESTEP_S) * SECONDS_TILL_STATS_DUMP;

    /** Whether network statistics logging is enabled or not. */
    bool netstatsLoggingEnabled;

    /** The number of ticks since we last logged our network statistics. */
    unsigned int ticksSinceNetstatsLog;
};

template<typename T>
void Network::serializeAndSend(const T& messageStruct)
{
    // Allocate the buffer.
    BinaryBufferSharedPtr messageBuffer
        = std::make_shared<BinaryBuffer>(Peer::MAX_WIRE_SIZE);

    // Serialize the message struct into the buffer, leaving room for the
    // headers.
    std::size_t messageSize
        = Serialize::toBuffer(*messageBuffer, messageStruct,
                              (CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE));

    // Check that the message isn't too big.
    const unsigned int totalMessageSize
        = CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE + messageSize;
    if ((totalMessageSize > Peer::MAX_WIRE_SIZE)
        || (messageSize > UINT16_MAX)) {
        LOG_ERROR("Tried to send a too-large message. Size: %u, max: %u",
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
    ByteTools::write16(messageSize, (messageBuffer->data() + CLIENT_HEADER_SIZE
                                     + MessageHeaderIndex::Size));

    // Shrink the buffer to fit.
    messageBuffer->resize(totalMessageSize);

    // Send the message.
    send(messageBuffer);
}

} // namespace Client
} // namespace AM
