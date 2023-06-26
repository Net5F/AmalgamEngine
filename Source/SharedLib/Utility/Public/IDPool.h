#pragma once

#include <vector>

namespace AM
{

/**
 * Provides unique identifiers.
 *
 * TODO: Re-use of IDs is an issue (e.g. if client 0 disconnects and another
 *       client connects and is given ID 0, there may be old messages in the
 *       queues that refer to ID 0 that will be incorrectly applied to the new
 *       client).
 *       The current soft solution is to reserve more IDs than we need, so
 *       that we don't immediately re-use the same ID. The real solution is to
 *       copy EnTT by making the top ~n bytes of our ID an "iteration", which
 *       gets incremented when the ID is freed.
 */
class IDPool
{
public:
    IDPool(std::size_t inPoolSize);

    /**
     * Reserves and returns the next empty ID.
     *
     * Marches forward, e.g. if 0-10 were reserved and freed, 11 will still be
     * the next reserved ID. After we reserve the last ID, we wrap back around.
     * This, along with SAFETY_BUFFER, aims to remove situations where an ID
     * was reserved, freed, and re-reserved while old data exists in the
     * system.
     */
    unsigned int reserveID();

    /**
     * Marks the given ID as reserved.
     */
    void markIDAsReserved(unsigned int ID);

    /**
     * Frees an ID for reuse.
     */
    void freeID(unsigned int ID);

    /**
     * Frees all IDs for reuse.
     */
    void freeAllIDs();

private:
    /** Extra room so that we don't run into reuse issues when almost all IDs
        are reserved.
        Note: If this isn't sufficient, you can just make your pool much
              larger than the number of IDs you plan on using. */
    static constexpr std::size_t SAFETY_BUFFER{100};

    /** The maximum number of IDs that we can give out. */
    std::size_t poolSize;

    /** The size of our container. Equal to poolSize + SAFETY_BUFFER. */
    std::size_t containerSize;

    /** The last index that we added an ID to. */
    int lastAddedIndex;

    /** The number of currently reserved IDs. */
    std::size_t reservedIDCount;

    /**
     * If ID 'x' is available, IDs[x] will be true. Else, it will be false.
     */
    std::vector<bool> IDs;
};

} // namespace AM
