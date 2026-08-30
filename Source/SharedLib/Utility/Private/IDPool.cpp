#include "IDPool.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
IDPool::IDPool(ReservationStrategy inStrategy, std::size_t initialPoolSize)
: strategy{inStrategy}
, reservedIDCount{0}
, nextMarchID{0}
, nextLowestID{0}
, IDs(initialPoolSize)
{
    // Make sure the initial size is > 0, otherwise our resizes will never
    // grow.
    if (initialPoolSize == 0) {
        IDs.resize(1);
    }
}

unsigned int IDPool::reserveID()
{
    // If we're out of IDs, double our capacity.
    if (reservedIDCount == IDs.size()) {
        IDs.resize(IDs.size() * 2);
    }

    if (strategy == ReservationStrategy::MarchForward) {
        // If the pool was previously full, find a newly allocated or freed ID.
        if (IDs[nextMarchID]) {
            setNextMarchID();
        }

        // Reserve nextMarchID.
        unsigned int returnID{nextMarchID};
        IDs[nextMarchID] = true;
        reservedIDCount++;

        // Find and set the next march ID.
        setNextMarchID();

        return returnID;
    }
    else if (strategy == ReservationStrategy::ReuseLowest) {
        // If the pool was previously full, find a newly allocated or freed ID.
        if (IDs[nextLowestID]) {
            setNextLowestID();
        }

        // Reserve nextLowestID.
        unsigned int returnID{nextLowestID};
        IDs[nextLowestID] = true;
        reservedIDCount++;

        // Find and set the next lowest ID.
        setNextLowestID();

        return returnID;
    }

    LOG_FATAL("Couldn't find a free ID when one should exist.");
    return 0;
}

void IDPool::markIDAsReserved(unsigned int ID)
{
    // If the ID isn't allocated, resize.
    if (ID >= IDs.size()) {
        IDs.resize(ID + 1);
    }

    IDs[ID] = true;
    reservedIDCount++;

    // If we're marching and this ID is the highest, find the next march ID.
    if ((strategy == ReservationStrategy::MarchForward)
        && (ID >= nextMarchID)) {
        setNextMarchID();
    }
    // Else if we're reusing lowest and this is the lowest ID, find the next
    // free ID.
    else if ((strategy == ReservationStrategy::ReuseLowest)
             && (ID == nextLowestID)) {
        setNextLowestID();
    }
}

void IDPool::freeID(unsigned int ID)
{
    if (ID >= IDs.size()) {
        LOG_FATAL("ID out of bounds: %u", ID);
        return;
    }

    // If the ID is reserved, free it.
    if (IDs[ID]) {
        IDs[ID] = false;
        reservedIDCount--;

        // If we're reusing lowest and this is the lowest ID, set it.
        if ((strategy == ReservationStrategy::ReuseLowest)
            && (ID < nextLowestID)) {
            nextLowestID = ID;
        }
    }
    else {
        LOG_FATAL("Tried to free an unused ID: %u", ID);
    }
}

void IDPool::freeAllIDs()
{
    std::fill(IDs.begin(), IDs.end(), false);
    reservedIDCount = 0;
    nextMarchID = 0;
    nextLowestID = 0;
}

void IDPool::setNextMarchID()
{
    // Growth is deferred until another ID is actually requested.
    if (reservedIDCount == IDs.size()) {
        return;
    }

    // March to the next ID, wrapping and searching if necessary.
    // Note: We add 1 to nextMarchID to avoid choosing the same one.
    for (std::size_t i{0}; i < (IDs.size() - 1); ++i) {
        std::size_t index{(nextMarchID + 1 + i) % IDs.size()};
        if (!IDs[index]) {
            nextMarchID = static_cast<unsigned int>(index);
            return;
        }
    }
}

void IDPool::setNextLowestID()
{
    // Growth is deferred until another ID is actually requested.
    if (reservedIDCount == IDs.size()) {
        return;
    }

    // Find the next available ID.
    // Note: We add 1 to nextLowestID to avoid choosing the same one.
    for (std::size_t i{nextLowestID + 1}; i < IDs.size(); ++i) {
        if (!IDs[i]) {
            nextLowestID = static_cast<unsigned int>(i);
            return;
        }
    }
}

} // namespace AM
