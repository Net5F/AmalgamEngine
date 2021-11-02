#pragma once

#include "DiscretePositionTools.h"

namespace AM
{
/**
 * The position of a particular tile.
 *
 * Note: Ideally this would be a strong alias of a DiscretePosition class, but
 *       there isn't a way to accomplish that without making the interface
 *       more clumsy, or losing our POD-type status.
 *       Instead, we've factored the hard-to-maintain logic out into a set of
 *       free functions that act on an isDiscretePosition concept.
 */
struct TilePosition {
    // Note: Screens and maps start at (0, 0) so we could make these unsigned,
    //       but these are signed to facilitate using this struct for things
    //       like negative offsets.
    /** The X-axis coordinate of this position. */
    int x{0};

    /** The Y-axis coordinate of this position. */
    int y{0};

    bool operator==(const TilePosition& other)
    {
        return (x == other.x) && (y == other.y);
    }

    bool operator!=(const TilePosition& other)
    {
        return (x != other.x) || (y != other.y);
    }

    TilePosition operator+(const TilePosition& other)
    {
        return {(x + other.x), (y + other.y)};
    }

    TilePosition& operator+=(const TilePosition& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    /**
     * If this position is within 1 tile of the given position, returns true.
     */
    bool isAdjacentTo(const TilePosition& other)
    {
        return DiscretePositionTools::isAdjacent(*this, other);
    }
};

} // namespace AM
