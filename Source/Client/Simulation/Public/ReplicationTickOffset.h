#pragma once

#include "Config.h"

namespace AM
{
namespace Client
{
/**
 * Represents a tick offset used while replicating non-predicted state, such
 * as NPC movement and tile map updates.
 *
 * This tick offset is negative, representing some point in the past.
 */
struct ReplicationTickOffset {
public:
    /**
     * Applies the given tick adjustment (received from the server) to
     * this offset.
     */
    void applyAdjustment(int adjustment);

    /**
     * Returns the value of this tick offset.
     */
    int get();

private:
    /** How far into the past to replicate at.
        e.g. If offset == -5, on tick 15 we'll replicate NPC data for tick 10.
        */
    int offset{Config::INITIAL_REPLICATION_OFFSET};
};

} // End namespace Client
} // End namespace AM
