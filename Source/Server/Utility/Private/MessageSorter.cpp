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
    if (!isTickValid(tickNum)) {
        // tickNum is invalid, release the lock.
        lock.unlock();
        DebugError("Tried to start receive for an invalid tick number.");
    }

    return queueBuffer[tickNum % BUFFER_SIZE];
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

Sint64 MessageSorter::push(Uint32 tickNum, BinaryBufferPtr message)
{
    // Acquire the lock.
    lock.lock();

    // If tickNum is valid, push the message.
    if (isTickValid(tickNum)) {
        queueBuffer[tickNum % BUFFER_SIZE].push(std::move(message));
    }

    // Release the lock.
    lock.unlock();

    // Return how far ahead or behind tickNum is in relation to currentTick.
    return static_cast<Sint64>(tickNum) - static_cast<Sint64>(currentTick);
}

bool MessageSorter::isTickValid(Uint32 tickNum)
{
    // Check if tickNum is within our lower and upper bounds.
    Uint32 upperBound = (currentTick + BUFFER_SIZE - 1);
    if ((tickNum < currentTick) || (tickNum > upperBound)) {
        return false;
    }
    else {
        return true;
    }
}

Uint32 MessageSorter::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM
