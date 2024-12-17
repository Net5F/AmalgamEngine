#pragma once

#include <vector>

namespace AM
{

/**
 * Provides unique identifiers.
 *
 * Note: Re-use of IDs is an issue (e.g. if client 0 disconnects and another
 *       client connects and is given ID 0, there may be old messages in the
 *       queues that refer to ID 0 that will be incorrectly applied to the new
 *       client).
 *       The current solution is to reserve more IDs than we need and always
 *       march forward, so that we don't immediately re-use the same ID.
 *       An alternative solution is to make the top ~n bits of our ID a
 *       "version", which gets incremented when the ID is freed. This is
 *       effectively the same as having extra IDs, though. The only reason to
 *       switch would be if we didn't want a static, pre-allocated pool size.
 */
class IDPool
{
public:
    /** The strategy to use when reserving IDs. */
    enum class ReservationStrategy {
         /** Marches forward, e.g. if 0-10 were reserved and freed, 11 will 
             still be the next reserved ID. After we reserve the last ID, we 
             wrap back around. This aims to remove situations where an ID was 
             reserved, freed, and re-reserved while old data exists in the 
             system. */
        MarchForward,
        ReuseLowest
    };

    /**
     * @param inStrategy The reservation strategy to use.
     * @param initialPoolSize The initial size of this ID pool. Must be > 0.
     *                        If the pool runs out of IDs, it will automatically
     *                        grow.
     */
    IDPool(ReservationStrategy inStrategy, std::size_t initialPoolSize);

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
    // Note: It'd be more ideal if we didn't allocate for new IDs until the ID 
    //       was actually needed. Our logic seems simpler if we do this "pre-
    //       find the next ID" approach, though.
    /**
     * Starting at nextMarchID, finds the next free ID, wrapping and searching 
     * if necessary.
     * @post nextMarchID is set to the next free ID to use. If no IDs are 
     *       available, the IDs vector will be resized to allocate more.
     */
    void setNextMarchID();

    /**
     * Starting at nextLowestID, searches for the next free ID.
     * @post nextLowestID is set to the next free ID to use. If no IDs are 
     *       available, the IDs vector will be resized to allocate more.
     */
    void setNextLowestID();

    /** The strategy to use when reserving IDs. */
    ReservationStrategy strategy;

    /** The number of currently reserved IDs. */
    std::size_t reservedIDCount;

    /** The next ID to use for the MarchForward strategy. */
    unsigned int nextMarchID;

    /** The lowest ID that is free for use, for the ReuseLowest strategy. */
    unsigned int nextLowestID;

    /** If ID 'x' is available, IDs[x] will be true. Else, it will be false. */
    std::vector<bool> IDs;
};

} // namespace AM
