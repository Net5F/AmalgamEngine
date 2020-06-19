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
    if (!isTickValid(tickNum)) {
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

Sint64 MessageSorter::push(Uint32 tickNum, BinaryBufferPtr message)
{
    // Acquire the mutex.
    mutex.lock();

    // If tickNum is valid, push the message.
    if (isTickValid(tickNum)) {
        queueBuffer[tickNum % BUFFER_SIZE].push(std::move(message));
    }

    // Release the mutex.
    mutex.unlock();

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
