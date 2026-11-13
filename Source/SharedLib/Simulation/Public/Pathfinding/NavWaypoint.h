#pragma once

#include "TilePosition.h"
#include "Vector3.h"

namespace AM
{

/**
 * A single point along a path, as returned by NavSearch.
 */
struct NavWaypoint {
    /** The node that the agent's footprint is anchored at (its northwest
        tile). */
    TilePosition tilePosition{};

    /** The world position of the bottom center of the agent's footprint, on
        the node's surface. */
    Vector3 position{};

    /** If true, the agent must jump to reach this waypoint from the previous
        one (i.e. it's climbing onto a higher surface). */
    bool isJump{false};
};

} // End namespace AM
