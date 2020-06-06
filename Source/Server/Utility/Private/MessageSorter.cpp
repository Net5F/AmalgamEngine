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

std::queue<BinaryBufferSharedPtr>& MessageSorter::startReceive(Uint32 tickNum)
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

bool MessageSorter::push(Uint32 tickNum, const BinaryBufferSharedPtr& message)
{
    // Acquire the lock.
    lock.lock();

    // Check if the tick is valid.
    if (!isTickValid(tickNum)) {
        // tickNum is invalid, release the lock.
        DebugInfo("Tick invalid. tickNum: %u, currentTick %u", tickNum, currentTick);
        lock.unlock();
        return false;
    }
    DebugInfo("Tick valid. tickNum: %u, currentTick %u. Pushing into slot: %u", tickNum,
        currentTick, (tickNum % BUFFER_SIZE));

    // Push the message.
    queueBuffer[tickNum % BUFFER_SIZE].push(message);

    // Release the lock.
    lock.unlock();

    return true;
}

bool MessageSorter::isTickValid(Uint32 tickNum)
{
    if (tickNum < currentTick || tickNum > (currentTick + BUFFER_SIZE - 1)) {
        // tickNum is invalid.
        return false;
    }
    else {
        return true;
    }
}

int MessageSorter::getCurrentTick()
{
    return currentTick;
}

} // namespace Server
} // namespace AM