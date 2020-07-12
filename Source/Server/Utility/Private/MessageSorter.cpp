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
    // Acquire the mutex.
    bool mutexWasFree = mutex.try_lock();
    if (!mutexWasFree) {
        DebugError(
            "Tried to startReceive twice in a row. You probably forgot to call endReceive.");
    }

    // Check if the tick is valid.
    if (isTickValid(tickNum) != ValidityResult::Valid) {
        // tickNum is invalid, release the lock.
        mutex.unlock();
        DebugError("Tried to start receive for an invalid tick number.");
    }

    return queueBuffer[tickNum % BUFFER_SIZE];
}

void MessageSorter::endReceive()
{
    if (mutex.try_lock()) {
        DebugError("Tried to release mutex while it isn't locked.");
    }

    // Advance the state.
    head++;
    currentTick++;

    // Release the mutex.
    mutex.unlock();
}

MessageSorter::PushResult MessageSorter::push(Uint32 tickNum, BinaryBufferPtr message)
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
    Sint64 diff = static_cast<Sint64>(tickNum) - static_cast<Sint64>(currentTick);

    // Release the mutex.
    mutex.unlock();

    return {validity, diff};
}

MessageSorter::ValidityResult MessageSorter::isTickValid(Uint32 tickNum)
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

Uint32 MessageSorter::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
