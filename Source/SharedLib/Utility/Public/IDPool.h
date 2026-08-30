#pragma once

#include <vector>

namespace AM
{

/**
 * Provides unique identifiers.
 *
 * Note: It's up to the caller to avoid reusing IDs. MarchForward reduces
 *       immediate reuse when other IDs are available, but callers that
 *       retain references to freed IDs should additionally use a "generation"
 *       or other stale-reference protection mechanism.
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
     * @param initialPoolSize The initial size of this ID pool. Must be > 0. If
     *                        another ID is requested after the pool runs
     *                        out, it will automatically grow.
     */
    IDPool(ReservationStrategy inStrategy, std::size_t initialPoolSize);

    /**
     * Reserves and returns the next empty ID.
     *
     * Marches forward, e.g. if 0-10 were reserved and freed, 11 will still be
     * the next reserved ID. After we reserve the last ID, we wrap back around.
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
    /**
     * Starting at nextMarchID, finds the next free ID, wrapping and searching
     * if necessary.
     * @post If an ID is available, nextMarchID is set to it. Otherwise,
     *       nextMarchID is left unchanged.
     */
    void setNextMarchID();

    /**
     * Starting at nextLowestID, searches for the next free ID.
     * @post If an ID is available, nextLowestID is set to it. Otherwise,
     *       nextLowestID is left unchanged.
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
