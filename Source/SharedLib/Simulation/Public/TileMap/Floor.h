#pragma once

#include "BoundingBox.h"

namespace AM
{
struct TilePosition;

/**
 * Definitions and helper functions for working with Floor tile layers.
 */
struct Floor {
    /**
     * Returns a bounding volume for a floor, translated to world space and 
     * offset to the given tile coords.
     */
    static BoundingBox calcWorldBounds(const TilePosition& tilePosition);
};

} // End namespace AM
