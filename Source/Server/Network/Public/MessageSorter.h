#pragma once

#include "NetworkDefs.h"
#include <SDL2/SDL_stdinc.h>
#include <array>
#include <queue>
#include <mutex>
#include "Log.h"

namespace AM
{
namespace Server
{
class MessageSorterBase
{
public:
    /** Indicates the validity of a given message's tick in relation to the
     * currentTick. */
    enum class ValidityResult {
        /** The given message's tick was less than the MessageSorter's
           currentTick. */
        TooLow,
        /** The message's tick is valid. */
        Valid,
        /** The given message's tick was beyond the end of the buffer. */
        TooHigh
    };
    /** The ValidityResult and associated diff from a push() operation. */
    struct PushResult {
        ValidityResult result;
        Sint64 diff;
    };
};

/**
 * A specialized container that sorts messages into an appropriate queue based
 * on the tick number they're associated with.
 *
 * Thread-safe, the intended usage is for an asynch receiver thread to act as
 * the producer, and for the main game loop to periodically consume the messages
 * for its current tick.
 *
 * To consume: Call startReceive, process all messages from the queue, then call
 * endReceive. Producer note: Will block on pushing until the consumer lock is
 * released.
 */
template<typename T>
class MessageSorter : public MessageSorterBase
{
public:
    typedef T value_type;

    /**
     * The max valid positive difference between an incoming tickNum and our
     * currentTick that we'll accept. If 10, the valid range is [currentTick,
     * currentTick + 10). Effectively, how far into the future we'll buffer
     * messages for.
     */
    static constexpr Sint64 BUFFER_SIZE = 10;

    /** The range of difference (inclusive) between a received message's tickNum
       and our currentTick that we'll push a message for. Outside of the bounds,
       we will drop the message. */
    static constexpr int MESSAGE_DROP_BOUND_LOWER = 0;
    static constexpr int MESSAGE_DROP_BOUND_UPPER = BUFFER_SIZE - 1;

    MessageSorter()
    : currentTick(0)
    , head(0)
    , isReceiving(false)
    {
    }

    /**
     * Starts a receive operation.
     *
     * NOTE: If tickNum is valid, locks the MessageSorter until endReceive is
     *       called.
     *
     * Returns a pointer to the queue holding messages for the given tick
     * number. If tickNum < currentTick or tickNum > (currentTick +
     * VALID_DIFFERENCE), it is considered not valid and an error occurs.
     *
     * @return If tickNum is valid (not too new or old), returns a pointer to a
     *         queue. Else, raises an error.
     */
    std::queue<value_type>& startReceive(Uint32 tickNum)
    {
        if (isReceiving) {
            LOG_ERROR("Tried to startReceive twice in a row. You probably "
                      "forgot to call endReceive.");
        }

        // Acquire the mutex.
        mutex.lock();

        // Check if the tick is valid.
        if (isTickValid(tickNum) != ValidityResult::Valid) {
            // tickNum is invalid, release the lock.
            mutex.unlock();
            LOG_ERROR("Tried to start receive for an invalid tick number.");
        }

        // Flag that we've started the receive operation.
        isReceiving = true;

        return queueBuffer[tickNum % BUFFER_SIZE];
    }

    /**
     * Ends an ongoing receive operation.
     *
     * NOTE: This function releases the lock set by startReceive.
     *
     * Increments currentTick and head, effectively removing the element at the
     * old currentTick and making messages at currentTick + BUFFER_SIZE - 1
     * valid to be pushed.
     *
     * @post The index previously pointed to by head is now head - 1,
     *       effectively making it the new end of the buffer.
     */
    void endReceive()
    {
        if (!isReceiving) {
            LOG_ERROR("Tried to endReceive() while not receiving.");
        }

        // Advance the state.
        head++;
        currentTick++;

        // Flag that we're ending the receive operation.
        isReceiving = false;

        // Release the mutex.
        mutex.unlock();
    }

    /**
     * If tickNum is valid, buffers the message.
     *
     * Note: Blocks on mutex if there's a receive ongoing.
     *
     * @return True if tickNum was valid and the message was pushed, else false.
     */
    PushResult push(Uint32 tickNum, value_type message)
    {
        /** Try to push the message. */
        // Acquire the mutex.
        mutex.lock();

        // Check validity of the message's tick.
        ValidityResult validity = isTickValid(tickNum);

        // If tickNum is valid, push the message.
        if (validity == ValidityResult::Valid) {
            queueBuffer[tickNum % BUFFER_SIZE].push(std::move(message));
        }

        // Calc the tick diff.
        Sint64 diff
            = static_cast<Sint64>(tickNum) - static_cast<Sint64>(currentTick);

        // Release the mutex.
        mutex.unlock();

        return {validity, diff};
    }

    /** Helper for checking if a tick number is within the bounds. */
    ValidityResult isTickValid(Uint32 tickNum)
    {
        // Check if tickNum is within our lower and upper bounds.
        Uint32 upperBound = (currentTick + BUFFER_SIZE - 1);
        if (tickNum < currentTick) {
            return ValidityResult::TooLow;
        }
        else if (tickNum > upperBound) {
            return ValidityResult::TooHigh;
        }
        else {
            return ValidityResult::Valid;
        }
    }

    /**
     * Returns the MessageSorter's internal currentTick.
     * NOTE: Should not be used to fetch the current tick, get a ref to the
     *       Game's currentTick instead. This is just for unit testing.
     */
    Uint32 getCurrentTick() { return currentTick; }

private:
    /**
     * Holds the queues used for sorting and storing messages.
     *
     * Holds messages at the index equal to their tick number - currentTick.
     * (e.g. if currentTick is 42, the queue in index 0 holds messages for tick
     * number 42, index 1: 43, ..., index (VALID_DIFFERENCE): (42 +
     * VALID_DIFFERENCE)).
     */
    std::array<std::queue<T>, BUFFER_SIZE> queueBuffer;

    /**
     * The current tick that we've advanced to.
     */
    Uint32 currentTick;

    /**
     * The index at which we're holding the current tick.
     */
    std::size_t head;

    /**
     * Used to prevent queueBuffer and head updates while a push or receive is
     * happening.
     */
    std::mutex mutex;

    /**
     * Tracks whether a receive operation has been started or not.
     * Note: Not thread safe. Only call from the same thread as startReceive()
     *       and endReceive().
     */
    bool isReceiving;

    //----------------------------------------------------------------------------
    // Convenience Functions
    //----------------------------------------------------------------------------
    /**
     * Returns the index, incremented by amount. Accounts for wrap-around.
     */
    std::size_t increment(const std::size_t index,
                          const std::size_t amount) const
    {
        return (index + amount) % BUFFER_SIZE;
    }
};

} // namespace Server
} // namespace AM
