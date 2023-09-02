#pragma once

#include "TilePosition.h"
#include "Position.h"
#include <array>
#include <cmath>

namespace AM
{
/**
 * Represents a world-space ray.
 *
 * A ray is a single point in space with a direction. It extends infinitely far
 * in that single direction (as opposed to a line, which extends in both 
 * directions).
 */
struct Ray {
    /** Origin point. */
    Position origin{};

    /** Direction vector. */
    float directionX{0};
    float directionY{0};
    float directionZ{0};

    /**
     * Normalizes this ray's current direction vector.
     */
    void normalize()
    {
        const float length{std::sqrt((directionX * directionX)
                                     + (directionY * directionY)
                                     + (directionZ * directionZ))};
        directionX /= length;
        directionY /= length;
        directionZ /= length;
    }

    /**
     * Returns the position along this ray at the given t.
     */
    Position getPositionAtT(float t)
    {
        return {(origin.x + (directionX * t)), (origin.y + (directionY * t)),
                (origin.z + (directionZ * t))};
    }
};

} // namespace AM
