#pragma once

#include "NetworkDefs.h"
#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Serialize.h"
#include "Peer.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "Log.h"
#include <SDL_stdinc.h>
#include <atomic>

namespace AM
{
namespace LTC
{
/**
 * Represents the Network for a single simulated client.
 *
 * We can't use the actual Client::Network class because it would spin up a 
 * thread for each client.
 */
class NetworkSimulation 
{
public:
    NetworkSimulation();

    /**
     * Attempts to connect to the server.
     */
    void connect();

    /**
     * Cleans up our server connection.
     */
    void disconnect();

    /**
     * Updates accumulatedTime. If greater than the tick timestep and no
     * messages have been sent since the last heartbeat, sends a message.
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
     * Attempts to receive messages from the server.
     */
    void receiveAndProcess();

    /**
     * Returns the Network event dispatcher. All messages that we receive
     * from the server are pushed into this dispatcher.
     */
    EventDispatcher& getEventDispatcher();

    /**
     * Returns the latest tick that we've received an update message for.
     */
    Uint32 getLastReceivedTick();

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

    std::shared_ptr<Peer> server;

    /** If true, the server connection has been established. */
    std::atomic<bool> serverConnected;

    /** Used to dispatch events from the network to the simulation. */
    EventDispatcher eventDispatcher;

    /** Deserializes messages, does any network-layer message handling, and
        pushes messages into the eventDispatcher. */
    Client::MessageProcessor messageProcessor;

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

    /** Holds a received server header while we process it. */
    BinaryBuffer headerRecBuffer;
    /** Holds a received message batch while we pass its messages to
        MessageProcessor. */
    BinaryBuffer batchRecBuffer;
    /** If a batch is compressed, it's decompressed into this buffer before
        processing. */
    BinaryBuffer decompressedBatchRecBuffer;
};

template<typename T>
void NetworkSimulation::serializeAndSend(const T& messageStruct)
{
    // Check that the message isn't too big.
    // Note: We don't compress messages on this side, so we know the final
    //       message size at this point.
    std::size_t totalMessageSize{CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE
                                 + Serialize::measureSize(messageStruct)};
    if (totalMessageSize > CLIENT_MAX_MESSAGE_SIZE) {
        LOG_INFO("Tried to send a too-large message. Size: %u, max: %u",
                  totalMessageSize, CLIENT_MAX_MESSAGE_SIZE);
        return;
    }

    // Serialize the message struct into a buffer, leaving room for the
    // headers.
    BinaryBufferSharedPtr messageBuffer{
        std::make_shared<BinaryBuffer>(totalMessageSize)};
    std::size_t messageSize{Serialize::toBuffer(
        messageBuffer->data(), messageBuffer->size(), messageStruct,
        (CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE))};

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

} // End namespace LTC
} // End namespace AM
