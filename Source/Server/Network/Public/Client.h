#pragma once

#include "Peer.h"
#include "NetworkDefs.h"
#include "Config.h"
#include "CircularBuffer.h"
#include "Timer.h"
#include "readerwriterqueue.h"
#include "Tracy.hpp"
#include <memory>
#include <array>
#include <mutex>
#include <atomic>

namespace AM
{
namespace Server
{
/**
 * This class represents a single client and facilitates the organization of our
 * communication with them.
 */
class Client
{
public:
    Client(NetworkID inNetID, std::unique_ptr<Peer> inPeer);

    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     *
     * @param message  The message to queue.
     * @param messageTick  If non-0, used to update our latestSentSimTick.
     *                     Use 0 if sending messages that aren't associated
     *                     with a tick.
     */
    void queueMessage(const BinaryBufferSharedPtr& message, Uint32 messageTick);

    /**
     * Attempts to send all queued messages over the network.
     *
     * @param currentTick  The sim's current tick.
     * @return An appropriate NetworkResult.
     */
    NetworkResult sendWaitingMessages(Uint32 currentTick);

    /**
     * Tries to receive a message from this client.
     * If no message is received, checks if this client has timed out.
     *
     * Note: It's expected that you called SDLNet_CheckSockets() on the
     *       outside-managed socket set before calling this.
     *
     * @param messageBuffer  The buffer to fill with a message, if one was
     *                       received.
     *
     * @return An appropriate ReceiveResult. If return.networkResult == Success,
     *         messageBuffer contains the received message.
     */
    ReceiveResult receiveMessage(Uint8* messageBuffer);

    /**
     * @return True if the client is connected, else false.
     *
     * Note: There's 2 places where a disconnect can occur:
     *       If the client initiates a disconnect, the peer will internally set
     *       a flag.
     *       If we initiated a disconnect, peer will be set to nullptr.
     *       Both cases are detected by this method.
     */
    bool isConnected();

    /**
     * Records the given tick diff in tickDiffHistory.
     *
     * Diffs represent how late a message was received (the difference between
     * the message's contained tick number and our current tick).
     *
     * If the diff isn't in our valid range, it's discarded and the client is
     * forcibly disconnected.
     */
    void recordTickDiff(Sint64 tickDiff);

    NetworkID getNetID();

private:
    //--------------------------------------------------------------------------
    // Helpers
    //--------------------------------------------------------------------------
    /**
     * Returns the number of messages waiting in the sendQueue.
     * The return type is Uint8 because it needs to fit in 1 byte of a message.
     */
    Uint8 getWaitingMessageCount() const;

    /**
     * Adds an explicit confirmation to the current batch.
     */
    void addExplicitConfirmation(unsigned int& currentIndex, Uint32 currentTick,
                                 Uint8& messageCount);

    /**
     * Compresses the first batchSize bytes in the payload section of 
     * batchBuffer into compressedBatchBuffer and returns the compressed 
     * payload size.
     */
    unsigned int compressBatch(unsigned int batchSize);

    /**
     * Fills in the header information for the message batch currently being
     * built.
     *
     * @param bufferToFill  The buffer that should have its header filled.
     * @param batchSize  The size, in bytes, of the current batch.
     * @param isCompressed  True if the batch is compressed, else false.
     */
    void fillHeader(Uint8* bufferToFill, Uint16 batchSize, bool isCompressed);

    //--------------------------------------------------------------------------
    // Connection, Batching
    //--------------------------------------------------------------------------
    /** Our Network-given ID. */
    const NetworkID netID;

    /** Our connection and interface to the client. */
    std::unique_ptr<Peer> peer;

    /** Convenience struct for passing data through the sendQueue. */
    struct QueuedMessage {
        /** The message to send. */
        BinaryBufferSharedPtr message;

        /** The tick that the message corresponds to. */
        Uint32 tick;
    };
    /** Holds messages to be sent with the next call to sendWaitingMessages. */
    moodycamel::ReaderWriterQueue<QueuedMessage> sendQueue;

    /** Holds header and message data while we're putting the next batch
        together.
        If the batch does not need to be compressed, it will be sent directly
        from this buffer. */
    static BinaryBuffer batchBuffer;

    /** If a batch needs to be compressed, the compressed bytes will be written
        to and sent from this buffer.
        See SharedConfig::BATCH_COMPRESSION_THRESHOLD for more info. */
    static BinaryBuffer compressedBatchBuffer;

    /** Tracks how long it's been since we've received a message from this
        client. */
    Timer receiveTimer;

    //--------------------------------------------------------------------------
    // Synchronization Functions
    //--------------------------------------------------------------------------
    struct AdjustmentData {
        /** The amount of adjustment. */
        Sint8 adjustment;
        /** The adjustment iteration that we're on. */
        Uint8 iteration;
    };

    /**
     * Calculates an appropriate tick adjustment for this client to make.
     * Increments adjustmentIteration every time it's called.
     */
    AdjustmentData getTickAdjustment();

    /**
     * Run through all checks necessary to determine if we should tell the
     * client to adjust its tick.
     *
     * Note: The parameters could be obtained internally, but passing them
     * in provides a clear separation between setup and adjustment logic.
     *
     * @param tickDiffHistoryCopy  A copy of the tickDiffHistory, so that we
     *                             don't need to keep it locked.
     * @param numFreshDiffsCopy  A copy of the numFreshDiffs, so that we don't
     *                           need to keep it locked.
     * @return The calculated adjustment. 0 if no adjustment was needed.
     */
    Sint8 calcAdjustment(CircularBuffer<Sint8, Config::TICKDIFF_HISTORY_LENGTH>&
                             tickDiffHistoryCopy,
                         unsigned int numFreshDiffsCopy);

    /**
     * Prints relevant information during an adjustment. Used for debugging.
     */
    void printAdjustmentInfo(
        const CircularBuffer<Sint8, Config::TICKDIFF_HISTORY_LENGTH>&
            tickDiffHistoryCopy,
        unsigned int numFreshDiffsCopy, int truncatedAverage);

    //--------------------------------------------------------------------------
    // Synchronization Members
    //--------------------------------------------------------------------------
    /** The latest tick that we've sent an update to this client for. */
    Uint32 latestSentSimTick;

    /**
     * Holds tick diffs that have been added through recordTickDiff.
     */
    CircularBuffer<Sint8, Config::TICKDIFF_HISTORY_LENGTH> tickDiffHistory;

    /**
     * Used to prevent tickDiffHistory and numFreshData from changing while a
     * getTickAdjustment() is happening.
     */
    TracyLockable(std::mutex, tickDiffMutex);

    /**
     * The number of fresh diffs that we have in the history.
     * Allows us to avoid using stale data when calculating adjustments.
     */
    std::atomic<unsigned int> numFreshDiffs;

    /**
     * Tracks the latest tick offset adjustment iteration we've received.
     * Looped around from the client to avoid double-counting adjustments.
     */
    std::atomic<Uint8> latestAdjIteration;
};

} // End namespace Server
} // End namespace AM
