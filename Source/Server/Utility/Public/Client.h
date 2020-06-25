#ifndef CLIENT_H_
#define CLIENT_H_

#include "GameDefs.h"
#include "NetworkDefs.h"
#include <memory>
#include <queue>
#include "CircularBuffer.h"
#include <mutex>
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
    Client(std::unique_ptr<Peer> inPeer);

    //--------------------------------------------------------------------------
    // Communication
    //--------------------------------------------------------------------------
    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     */
    void queueMessage(const BinaryBufferSharedPtr& message);

    /**
     * Immediately sends the given header to this Peer.
     *
     * Could technically send any message, but our use case is only for headers. All other
     * messages should be queued through queueMessage.
     *
     * Will error if the message size is larger than a Uint16 can hold.
     * @return Disconnected if the peer was found to be disconnected, else Success.
     */
    NetworkResult sendHeader(const BinaryBufferSharedPtr& header);

    /**
     * Attempts to send all queued messages over the network.
     * @return false if the client was found to be disconnected, else true.
     */
    NetworkResult sendWaitingMessages();

    /**
     * Tries to receive a message from the Peer.
     * Note: It's expected that you called SDLNet_CheckSockets() on the outside-managed
     *       socket set before calling this.
     *
     * @return An appropriate ReceiveResult if the receive failed, else a ReceiveResult with
     *         result == Success and data in the message field.
     */
    ReceiveResult receiveMessage();

    /**
     * Returns the number of messages waiting in sendQueue.
     * The return type is Uint8 because it needs to fit in 1 byte of a message.
     */
    Uint8 getWaitingMessageCount() const;

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
    /** The average difference that we'll aim a client towards. */
    static constexpr Sint64 TARGET_TICKDIFF = 3;

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
    /** How long we should wait before considering the client to be timed out. */
    static constexpr float TIMEOUT_S = NETWORK_TICK_INTERVAL_S * 2;

    /**
     * Our connection and interface to the client.
     */
    std::unique_ptr<Peer> peer;

    /**
     * Holds messages to be sent with the next call to sendWaitingMessages.
     */
    std::deque<BinaryBufferSharedPtr> sendQueue;

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
     * An iteration count of tick offset adjustments that we've sent to this client.
     * Used by the client to avoid double-counting adjustments.
     */
    Uint8 adjustmentIteration;

    /** Tracks how long it's been since we've received a message from this client. */
    Timer receiveTimer;
};

} // End namespace Server
} // End namespace AM

#endif /* End CLIENT_H_ */
