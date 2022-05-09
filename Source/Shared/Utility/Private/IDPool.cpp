#include "IDPool.h"
#include "Log.h"

namespace AM
{
IDPool::IDPool(unsigned int inPoolSize)
: poolSize(inPoolSize)
, containerSize(poolSize + SAFETY_BUFFER)
, lastAddedIndex(0)
, reservedIDCount(0)
, IDs(containerSize)
{
}

unsigned int IDPool::reserveID()
{
    if (reservedIDCount > poolSize) {
        LOG_FATAL("Tried to reserve ID when all were taken.");
        return 0;
    }

    // Find the next empty index.
    for (unsigned int i = 1; i < poolSize; ++i) {
        // If this index is false (ID is unused).
        unsigned int index{(lastAddedIndex + i) % containerSize};
        if (!IDs[index]) {
            IDs[index] = true;
            lastAddedIndex = index;
            reservedIDCount++;

            return index;
        }
    }

    LOG_FATAL("Couldn't find an empty index when one should exist.");
    return 0;
}

void IDPool::freeID(unsigned int ID)
{
    if (IDs[ID]) {
        IDs[ID] = false;
        reservedIDCount--;
    }
    else {
        LOG_FATAL("Tried to free an unused ID.");
    }
}

} // namespace AM
