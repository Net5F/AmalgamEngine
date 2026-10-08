#pragma once

#include "Peer.h"
#include "NetworkID.h"
#include "BufferPool.h"
#include "Config.h"
#include "CircularBuffer.h"
#include "Timer.h"
#include "readerwriterqueue.h"
#include "tracy/Tracy.hpp"
#include <memory>
#include <array>
#include <mutex>
#include <atomic>
#include <span>

namespace AM
{
namespace WorldServer
{
/**
 * Represents a single networked client.
 *
 * Manages sending/receiving messages, and connection state.
 */
class Client
{
public:
    /** How long a client has to authenticate after connecting, before we
        drop it.
        Note: If a project ever cares to configure this, it can be moved into
              Config.h. */
    static constexpr double AUTH_TIMEOUT_S{10};

    enum class AuthState {
        /** We're waiting for the client to send a ConnectionRequest. */
        AwaitingRequest,
        /** We're waiting for the AccountServer to validate the client's
            ticket. */
        Validating,
        /** The client's ticket was rejected. If the client doesn't
            disconnect, it'll be dropped when AUTH_TIMEOUT_S expires. */
        Rejected,
        /** The client's ticket was accepted and it was added to the sim. */
        Authenticated
    };

    Client(NetworkID inNetID, std::unique_ptr<Peer> inPeer);

    /**
     * Checks if this client has timed out (or failed to authenticate in
     * time), then returns its connection state.
     * @return true if the client is connected, else false.
     *
     * Note: There's 2 places where a disconnect can occur:
     *       If the client initiates a disconnect, the peer will internally set
     *       a flag.
     *       If we initiated a disconnect, peer will be set to nullptr.
     *       Both cases are detected by this method.
     */
    bool isConnected();

    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     *
     * @param message The message to queue.
     * @param messageTick If non-0, used to update our latestSentSimTick.
     *                    Use 0 if sending messages that aren't associated
     *                    with a tick.
     */
    void queueMessage(const BinaryBufferSharedPtr& message, Uint32 messageTick);

    /**
     * Attempts to send all queued messages over the network.
     *
     * @param currentTick The sim's current tick.
     * @return An appropriate NetworkResult.
     */
    NetworkResult sendWaitingMessages(Uint32 currentTick);

    /**
     * Returns true if this socket has data waiting.
     * Only valid if clientSet->checkSockets() is called prior.
     */
    bool dataIsReady();

    struct ReceiveResult {
        /** The result of the receive attempt. */
        NetworkResult networkResult{NetworkResult::NotSet};
        /** The type of the received message. Will be NotSet if networkResult
            != Success.
            Note: This gets cast to EngineMessageType or ProjectMessageType. */
        Uint8 messageType{static_cast<Uint8>(EngineMessageType::NotSet)};
        /** If networkResult == Success, contains the received message. */
        std::span<Uint8> messageBuffer{};
    };
    /**
     * Blocks until a message is received from this client.
     *
     * Note: Use dataIsReady() to check for data before calling this.
     *
     * @return An appropriate ReceiveResult. If return.networkResult == Success,
     *         messageBuffer contains the received message.
     */
    ReceiveResult receiveMessage();

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

    NetworkID getNetID() const;

    /**
     * Note: The auth state should only be accessed from the receive thread.
     */
    AuthState getAuthState() const;
    void setAuthState(AuthState inAuthState);

    /**
     * Note: The account ID should only be accessed from the receive thread.
     *       It's only valid once this client is Authenticated.
     */
    Sint64 getAccountID() const;
    void setAccountID(Sint64 inAccountID);

    /**
     * Forcibly disconnects this client. It'll be erased the next time
     * isConnected() is checked.
     */
    void disconnect();

private:
    //--------------------------------------------------------------------------
    // Helpers
    //--------------------------------------------------------------------------
    /**
     * Returns the number of messages waiting in the sendQueue.
     */
    std::size_t getWaitingMessageCount() const;

    /**
     * Adds an explicit confirmation to the current batch.
     */
    void addExplicitConfirmation(std::size_t& currentIndex, Uint32 currentTick);

    /**
     * Compresses the first batchSize bytes in the payload section of
     * batchBuffer into compressedBatchBuffer and returns the compressed
     * payload size.
     */
    std::size_t compressBatch(std::size_t batchSize);

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

    //--------------------------------------------------------------------------
    // Batching/Sending
    //--------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------
    // Receiving
    //--------------------------------------------------------------------------
    /** The max size that a received client message can be before we need to
        anticipate that it will be split into multiple packets over the wire. */
    static constexpr std::size_t CLIENT_MAX_SMALL_MESSAGE_SIZE{
        ETHERNET_MTU - CLIENT_HEADER_SIZE - MESSAGE_HEADER_SIZE};

    /** Holds any small (single packet) received messages.
        For small messages, we can use a single buffer for all clients since we
        know all of the data will be immediately present. */
    static BinaryBuffer smallReceiveBuffer;

    /** A pool that holds all the buffers we've allocated for large message
        receiving. */
    using LargeBufferPool = BufferPool<CLIENT_MAX_MESSAGE_SIZE>;
    static LargeBufferPool bufferPool;

    /** If nullptr, we're currently composing a large message into this buffer.
     */
    std::unique_ptr<LargeBufferPool::BufferType> largeReceiveBuffer;

    /** The type of the message that we're currently composing. */
    Uint8 messageType;

    /** The size of the message that we're currently composing.
        Note: This eventually gets cast to EngineMessageType or
              ProjectMessageType. */
    Uint16 messageSize;

    /** If we're currently composing a large message, this is the first empty
        index within largeReceiveBuffer. */
    Uint16 compositionIndex;

    /** Tracks how long it's been since we've received a message from this
        client. */
    Timer receiveTimer;

    //--------------------------------------------------------------------------
    // Authentication
    //--------------------------------------------------------------------------
    /** This client's current authentication state. */
    AuthState authState;

    /** The ID of the account that this client authenticated as. */
    Sint64 accountID;

    /** Tracks how long it's been since this client connected. Used to drop
        clients that don't authenticate within AUTH_TIMEOUT_S. */
    Timer authTimer;

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

} // End namespace WorldServer
} // End namespace AM
