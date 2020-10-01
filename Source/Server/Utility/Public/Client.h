#pragma once

#include "GameDefs.h"
#include "NetworkDefs.h"
#include "Peer.h"
#include <memory>
#include <queue>
#include <array>
#include "CircularBuffer.h"
#include <mutex>
#include <atomic>
#include "Timer.h"

namespace AM
{

class Peer;

namespace Server
{

/**
 * This class represents a single client and facilitates the organization of our
 * communication with them.
 *
 * It's effectively an adapter for Peer with an added outgoing queue and some
 * storing of sync data.
 */
class Client
{
public:
    Client(NetworkID inNetID, std::unique_ptr<Peer> inPeer);

    //--------------------------------------------------------------------------
    // Communication
    //--------------------------------------------------------------------------
    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     */
    void queueMessage(const BinaryBufferSharedPtr& message);

    /**
     * Attempts to send all queued messages over the network.
     * @param currentTick  The sim's current tick.
     * @return false if the client was found to be disconnected, else true.
     */
    NetworkResult sendWaitingMessages(Uint32 currentTick);

    /**
     * Tries to receive a message from this client.
     * If no message is received, checks if this client has timed out.
     * Note: It's expected that you called SDLNet_CheckSockets() on the outside-managed
     *       socket set before calling this.
     *
     * @return A received Message. If no data was waiting, return.messageType will
     *         == NotSet and return.messageBuffer will == null.
     */
    Message receiveMessage();

    /**
     * @return True if the client is connected, else false.
     *
     * Note: There's 2 places where a disconnect can occur:
     *       If the client initiates a disconnect, the peer will internally set a flag.
     *       If we initiated a disconnect, peer will be set to nullptr.
     *       Both cases are detected by this method.
     */
    bool isConnected();

    //--------------------------------------------------------------------------
    // Synchronization
    //--------------------------------------------------------------------------
    /** The number of tick offsets that we'll remember. */
    static constexpr int TICKDIFF_HISTORY_LENGTH = 10;

    /** The lowest difference we'll work with. */
    static constexpr Sint64 LOWEST_VALID_TICKDIFF = -10;
    /** The highest difference we'll work with. */
    static constexpr Sint64 HIGHEST_VALID_TICKDIFF = 10;
    /** The range of difference (inclusive) between a received message's tickNum and our
        current tickNum that we won't send an adjustment for. */
    static constexpr Sint64 TICKDIFF_ACCEPTABLE_BOUND_LOWER = 1;
    static constexpr Sint64 TICKDIFF_ACCEPTABLE_BOUND_UPPER = 3;
    /** The value that we'll adjust clients to if they fall outside the bounds. */
    static constexpr Sint64 TICKDIFF_TARGET = 2;

    struct AdjustmentData {
        /** The amount of adjustment. */
        Sint8 adjustment;
        /** The adjustment iteration that we're on. */
        Uint8 iteration;
    };

    /**
     * Records the given tick diff in tickDiffHistory.
     *
     * Diffs are generally obtained through the return value of MessageSorter.push(),
     * and represent how far off the tickNum in a received message was from our current tick.
     *
     * If the diff isn't in our valid range, it's discarded and the client is forcibly
     * disconnected.
     */
    void recordTickDiff(Sint64 tickDiff);

    /**
     * Calculates an appropriate tick adjustment for this client to make.
     * Increments adjustmentIteration every time it's called.
     */
    AdjustmentData getTickAdjustment();

private:
    //--------------------------------------------------------------------------
    // Private Functions
    //--------------------------------------------------------------------------
    /**
     * Returns the number of messages waiting in the sendQueue.
     * The return type is Uint8 because it needs to fit in 1 byte of a message.
     */
    Uint8 getWaitingMessageCount() const;

    /**
     * Run through all checks necessary to determine if we should tell the client to adjust
     * its tick.
     * Note: The parameters could be obtained internally, but passing them in provides a
     *       clear separation between setup and adjustment logic.
     * @param averageDiff  The average tick difference, calculated from the tickDiffHistory.
     * @param tickDiffHistoryCopy  A copy of the tickDiffHistory, so that we don't need to
     *        lock it.
     * @return The calculated adjustment. 0 if no adjustment was needed.
     */
    Sint8 calcAdjustment(
    float averageDiff,
    CircularBuffer<Sint8, TICKDIFF_HISTORY_LENGTH>& tickDiffHistoryCopy);

    //--------------------------------------------------------------------------
    // Connection, Batching
    //--------------------------------------------------------------------------
    /** How long we should wait before considering the client to be timed out. */
    static constexpr double TIMEOUT_S = NETWORK_TICK_TIMESTEP_S * 10;

    /** Our Network-given ID. */
    NetworkID netID;

    /**
     * Our connection and interface to the client.
     */
    std::unique_ptr<Peer> peer;

    /**
     * Holds messages to be sent with the next call to sendWaitingMessages.
     */
    std::deque<BinaryBufferSharedPtr> sendQueue;

    /** Holds data while we're putting it together to be sent as a batch. */
    std::array<Uint8, Peer::MAX_MESSAGE_SIZE> batchBuffer;

    /** Tracks how long it's been since we've received a message from this client. */
    Timer receiveTimer;

    //--------------------------------------------------------------------------
    // Synchronization
    //--------------------------------------------------------------------------
    /**
     * Holds tick diffs that have been added through recordTickDiff.
     */
    CircularBuffer<Sint8, TICKDIFF_HISTORY_LENGTH> tickDiffHistory;

    /**
     * Used to prevent tickDiffHistory changing while a getTickAdjustment is happening.
     */
    std::mutex tickDiffMutex;

    /** Used to flag that we've recorded a tick diff. */
    bool hasRecordedDiff;

    /**
     * Tracks the latest tick offset adjustment iteration we've received.
     * Looped around from the client to avoid double-counting adjustments.
     */
    std::atomic<Uint8> latestAdjIteration;
};

} // End namespace Server
} // End namespace AM
