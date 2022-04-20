#pragma once

#include "NetworkDefs.h"
#include <SDL_stdinc.h>
#include <array>
#include <queue>
#include "Log.h"

namespace AM
{
namespace Server
{
/**
 * Base class so we don't have to use EventSorter<ClassName>:: to reference
 * enums and such.
 */
class SorterBase
{
public:
    /**
     * Indicates the validity of a given event's tick in relation to the
     * currentTick.
     */
    enum class ValidityResult {
        /** The given message's tick was less than the EventSorter's
            currentTick. */
        TooLow,
        /** The event's tick is valid. */
        Valid,
        /** The given event's tick was beyond the end of the buffer. */
        TooHigh
    };
};

/**
 * A specialized container that sorts events into an appropriate queue based
 * on the tick number they're associated with.
 *
 * Events need to be sorted instead of directly processing them out of the
 * queue, because we may receive them out of order from clients. E.g.,
 * if we're on tick 39 and we receive a message with tickNum = 42 from client 1,
 * and another with tickNum = 40 from client 2.
 *
 * Not thread safe, use an EventQueue first to move events across threads.
 */
template<typename T>
class EventSorter : public SorterBase
{
public:
    /**
     * The max valid positive difference between an incoming tickNum and our
     * currentTick that we'll accept. If 10, the valid range is [currentTick,
     * currentTick + 10). Effectively, how far into the future we'll buffer
     * messages for.
     */
    static constexpr Sint64 BUFFER_SIZE = 10;

    EventSorter()
    : currentTick(0)
    {
    }

    /**
     * Returns a pointer to the queue holding events for the current tick.
     */
    std::queue<T>* getCurrentQueue()
    {
        return &(queueBuffer[currentTick % BUFFER_SIZE]);
    }

    /**
     * Advances this sorter to the next tick.
     */
    void advance()
    {
        // Advance the state.
        currentTick++;
    }

    /**
     * If tickNum is valid, buffers the message.
     *
     * @return True if tickNum was valid and the message was pushed, else false.
     */
    ValidityResult push(const T& event, Uint32 tickNum)
    {
        // Check validity of the event's tick.
        ValidityResult validity = isTickValid(tickNum);

        // If tickNum is valid, push the event.
        if (validity == ValidityResult::Valid) {
            queueBuffer[tickNum % BUFFER_SIZE].push(std::move(event));
        }

        return validity;
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
     * Returns the EventSorter's internal currentTick.
     *
     * NOTE: Should not be used to fetch the current tick, get a ref to the
     *       Simulation's currentTick instead. This is just to see where the
     *       sorter is at.
     */
    Uint32 getCurrentTick() { return currentTick; }

private:
    /**
     * Holds the queues used for sorting and storing events.
     *
     * Holds events at an index equal to their tick number % BUFFER_SIZE.
     */
    std::array<std::queue<T>, BUFFER_SIZE> queueBuffer;

    /**
     * The current tick that we've advanced to.
     */
    Uint32 currentTick;
};

} // namespace Server
} // namespace AM
