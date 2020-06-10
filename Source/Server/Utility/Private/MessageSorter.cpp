#include "MessageSorter.h"
#include "Debug.h"

namespace AM
{
namespace Server
{

MessageSorter::MessageSorter()
: currentTick(0)
, head(0)
{
}

std::queue<BinaryBufferPtr>& MessageSorter::startReceive(Uint32 tickNum)
{
    // Acquire the lock.
    bool lockWasFree = lock.try_lock();
    if (!lockWasFree) {
        DebugError(
            "Tried to startReceive twice in a row. You probably forgot to call endReceive.");
    }

    // Check if the tick is valid.
    Uint32 upperBound = (currentTick + VALID_DIFFERENCE - 1);
    if ((tickNum < currentTick) || (tickNum > upperBound)) {
        // tickNum is invalid, release the lock.
        lock.unlock();
        DebugError("Tried to start receive for an invalid tick number.");
    }

    return queueBuffer[tickNum % VALID_DIFFERENCE];
}

void MessageSorter::endReceive()
{
    if (lock.try_lock()) {
        DebugError("Tried to release lock while it isn't locked.");
    }

    // Advance the state.
    head++;
    currentTick++;

    // Release the lock.
    lock.unlock();
}

Sint32 MessageSorter::push(Uint32 tickNum, BinaryBufferPtr message)
{
    // Acquire the lock.
    lock.lock();

    // Calc the difference (may underflow).
    Uint32 difference = tickNum - currentTick;
    Sint32 returnValue = INVALID_VALUE;

    /* Carefully process the difference. */
    if (difference <= VALID_DIFFERENCE) {
        // Positive difference within range.
        returnValue = static_cast<Sint32>(difference);

        // Push the message.
        queueBuffer[tickNum % VALID_DIFFERENCE].push(std::move(message));
    }
    else {
        // Either positive but too large, or negative (underflowed during subtraction).

        // Check if it's within the acceptable negative range by trying to overflow
        // back around.
        if ((difference + VALID_DIFFERENCE) >= 0) {
            // Acceptable negative difference.
            returnValue = static_cast<Sint32>(difference);
        }
    }

    // Release the lock.
    lock.unlock();

    // Return either INVALID_VALUE, or the valid difference.
    return returnValue;
}

Uint32 MessageSorter::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
