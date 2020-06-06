#ifndef MESSAGESORTER_H_
#define MESSAGESORTER_H_

#include "NetworkDefs.h"
#include <array>
#include <queue>
#include <mutex>

namespace AM
{
namespace Server
{

/**
 * A specialized container that sorts messages into an appropriate queue based on the tick
 * number they're associated with.
 *
 * Thread-safe, the intended usage is for an asynch receiver thread to act as the producer,
 * and for the main game loop to periodically consume the messages for its current tick.
 *
 * To consume: Call startReceive, process all messages from the queue, then call endReceive.
 * Producer note: Will block on pushing until the consumer lock is released.
 */
class MessageSorter
{
public:
    /**
     * The number of ticks in the future that we'll accept messages for.
     * If equal to 10 and currentTick is 42, the range [42, 51] is valid.
     */
    static constexpr unsigned int BUFFER_SIZE = 10;

    MessageSorter();

    /**
     * Returns a pointer to the queue holding messages for the given tick number.
     * If tickNum < currentTick or tickNum >= (currentTick + BUFFER_SIZE), it is considered
     * not valid and an error occurs.
     *
     * NOTE: If tickNum is valid, locks the MessageSorter until endReceive is called.
     *
     * @return If tickNum is valid (not too new or old), returns a pointer to a queue.
     *         Else, raises an error.
     */
    std::queue<BinaryBufferSharedPtr>& startReceive(Uint32 tickNum);

    /**
     * Increments currentTick and head, effectively removing the element at the old
     * currentTick and making messages at currentTick + BUFFER_SIZE - 1 valid to
     * be pushed.
     *
     * NOTE: This function releases the lock set by startReceive.
     *
     * @post the index previously pointed to by head is now head - 1, effectively making it
     * the new end of the buffer.
     */
    void endReceive();

    /**
     * If tickNum is valid, pushes the given message into the appropriate queue.
     *
     * Note: Blocks while a receive has been started, until the receive is ended.
     *
     * @return true if tickNum was valid and the message was pushed, else false.
     */
    bool push(Uint32 tickNum, const BinaryBufferSharedPtr& message);

    /** Helper for checking if a tick number is within the bounds. */
    bool isTickValid(Uint32 tickNum);

    int getCurrentTick();

private:
    /**
     * Holds the queues used for sorting and storing messages.
     *
     * Holds messages at the index equal to their tick number - currentTick.
     * (e.g. if currentTick is 42, the queue in index 0 holds messages for tick number 42,
     * index 1: 43, ..., index (BUFFER_SIZE - 1): (currentTick + BUFFER_SIZE - 1)).
     */
    std::array<std::queue<BinaryBufferSharedPtr>, BUFFER_SIZE> queueBuffer;

    /**
     * The current tick that we've advanced to.
     */
    Uint32 currentTick;

    /**
     * The index at which we're holding the current tick.
     */
    std::size_t head;

    /**
     * Used to lock the container while a push or receive is happening.
     */
    std::mutex lock;

    //----------------------------------------------------------------------------
    // Convenience Functions
    //----------------------------------------------------------------------------
    /**
     * Returns the index, incremented by amount. Accounts for wrap-around.
     */
    std::size_t increment(const std::size_t index, const std::size_t amount) const
    {
        return (index + amount) % BUFFER_SIZE;
    }
};

} // namespace Server
} // namespace AM

#endif /* MESSAGESORTER_H_ */
