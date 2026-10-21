#pragma once

#include "WorldClientConnection.h"
#include "ConnectionHandle.h"
#include "NetworkID.h"
#include "DisconnectNotice.h"
#include "BinaryBuffer.h"
#include "Config.h"
#include "CircularBuffer.h"
#include <SDL3/SDL_stdinc.h>
#include <memory>
#include <optional>
#include <vector>

namespace AM
{
namespace WorldServer
{
class WorldBatchFramer;

/**
 * Represents a single networked client.
 *
 * Holds the client's connection, along with its authentication and tick
 * synchronization state.
 *
 * Note: This class is only accessed from the network IO thread.
 */
class Client
{
public:
    enum class AuthState {
        /** We're waiting for the client to send a ConnectionRequest. */
        AwaitingRequest,
        /** We're waiting for AccountServer to validate the client's
            ticket. */
        Validating,
        /** The client's ticket was rejected. We're closing the connection
            once the client receives our ConnectionResponse. */
        Rejected,
        /** The client's ticket was accepted and it was added to the sim. */
        Authenticated
    };

    Client(std::shared_ptr<WorldClientConnection> inConnection);

    /**
     * Closes our connection without invoking its disconnect callback.
     * Note: Used by ConnectionRegistry.
     */
    void close();

    void setHandle(ConnectionHandle inHandle);
    ConnectionHandle getHandle() const;

    /**
     * Returns this client's network ID.
     * Note: This is the ID of the client's registry slot.
     */
    NetworkID getNetID() const;

    WorldClientConnection& getConnection();

    AuthState getAuthState() const;
    void setAuthState(AuthState inAuthState);

    /**
     * Note: The account ID is only valid once this client is Authenticated.
     */
    Sint64 getAccountID() const;
    void setAccountID(Sint64 inAccountID);

    /**
     * Returns true if we've started disconnecting this client.
     * Once disconnecting, we no longer send or process messages.
     */
    bool isDisconnecting() const;
    void setDisconnecting();

    /**
     * Processes the adjustment iteration from a received message's header.
     *
     * @return true if the iteration was valid, else false.
     */
    bool processAdjustmentIteration(Uint8 receivedIteration);

    /**
     * Records the given tick diff in tickDiffHistory.
     *
     * Diffs represent how late a message was received (the difference between
     * the message's contained tick number and our current tick).
     *
     * @return true if the diff was in our valid range, else false (the caller
     *         should disconnect the client).
     */
    bool recordTickDiff(Sint64 tickDiff);

    /**
     * Queues a message to be sent the next time sendQueuedMessages() is
     * called.
     *
     * @param message The message to queue.
     * @param messageTick If non-0, used to update our latestSentSimTick.
     *                    Use 0 if sending messages that aren't associated
     *                    with a tick.
     */
    void queueMessage(BinaryBufferSharedPtr message, Uint32 messageTick);

    /**
     * Queues a DisconnectNotice to be sent after any queued messages. Any
     * messages queued afterwards are dropped.
     */
    void queueDisconnect(DisconnectNotice::Reason reason);

    /**
     * Returns the reason passed to queueDisconnect(), if it was called.
     */
    std::optional<DisconnectNotice::Reason> getQueuedDisconnect() const;

    /**
     * Frames all queued messages into batches and sends them, along with any
     * necessary explicit confirmation or DisconnectNotice.
     *
     * @param framer The framer to build the batches with.
     * @param currentTick The sim's current tick.
     * @return The number of bytes that were sent.
     */
    std::size_t sendQueuedMessages(WorldBatchFramer& framer,
                                   Uint32 currentTick);

private:
    struct QueuedMessage {
        /** The message to send. */
        BinaryBufferSharedPtr message{};

        /** The tick that the message corresponds to. */
        Uint32 tick{0};
    };

    struct AdjustmentData {
        /** The amount of adjustment. */
        Sint8 adjustment;
        /** The adjustment iteration that we're on. */
        Uint8 iteration;
    };

    /**
     * Appends the given message to the framer's batch. If it doesn't fit,
     * sends the current batch first (and sets batchWasSent).
     *
     * @return The number of bytes that were sent.
     */
    template<typename Message>
    std::size_t appendMessage(WorldBatchFramer& framer, const Message& message,
                              const AdjustmentData& adjustmentData,
                              bool& batchWasSent);

    /**
     * Finishes the framer's current batch and sends it.
     *
     * @return The number of bytes that were sent.
     */
    std::size_t sendBatch(WorldBatchFramer& framer,
                          const AdjustmentData& adjustmentData);

    /**
     * Calculates an appropriate tick adjustment for this client to make.
     */
    AdjustmentData getTickAdjustment();

    /**
     * Run through all checks necessary to determine if we should tell the
     * client to adjust its tick.
     *
     * @return The calculated adjustment. 0 if no adjustment was needed.
     */
    Sint8 calcAdjustment();

    /**
     * Prints relevant information during an adjustment. Used for debugging.
     */
    void printAdjustmentInfo(int truncatedAverage);

    //--------------------------------------------------------------------------
    // Connection
    //--------------------------------------------------------------------------
    std::shared_ptr<WorldClientConnection> connection;

    /** Our registry handle. Our network ID is its slot ID. */
    ConnectionHandle handle;

    /** If true, we've started disconnecting this client. */
    bool disconnecting;

    //--------------------------------------------------------------------------
    // Authentication
    //--------------------------------------------------------------------------
    /** This client's current authentication state. */
    AuthState authState;

    /** The ID of the account that this client authenticated as. */
    Sint64 accountID;

    //--------------------------------------------------------------------------
    // Sending
    //--------------------------------------------------------------------------
    /** Holds messages to be sent with the next call to sendQueuedMessages().
        Note: Only used during a flush, but kept as a member so its capacity
              is reused. */
    std::vector<QueuedMessage> queuedMessages;

    /** If set, a DisconnectNotice will be sent after the queued messages. */
    std::optional<DisconnectNotice::Reason> queuedDisconnect;

    /** The latest tick that we've sent an update to this client for. */
    Uint32 latestSentSimTick;

    //--------------------------------------------------------------------------
    // Synchronization
    //--------------------------------------------------------------------------
    /** Holds tick diffs that have been added through recordTickDiff(). */
    CircularBuffer<Sint8, Config::TICKDIFF_HISTORY_LENGTH> tickDiffHistory;

    /**
     * The number of fresh diffs that we have in the history.
     * Allows us to avoid using stale data when calculating adjustments.
     */
    unsigned int numFreshDiffs;

    /**
     * Tracks the latest tick offset adjustment iteration we've received.
     * Looped around from the client to avoid double-counting adjustments.
     */
    Uint8 latestAdjIteration;
};

} // End namespace WorldServer
} // End namespace AM
