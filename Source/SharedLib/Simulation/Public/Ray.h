#pragma once

#include "Vector3.h"
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
    Vector3 origin{};

    /** Direction vector. */
    Vector3 direction{};

    /**
     * Normalizes this ray's current direction vector.
     */
    void normalize()
    {
        const float length{std::sqrt((direction.x * direction.x)
                                     + (direction.y * direction.y)
                                     + (direction.z * direction.z))};
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;
    }

    /**
     * Returns the point along this ray at the given t.
     */
    Vector3 getPointAtT(float t)
    {
        return {(origin.x + (direction.x * t)), (origin.y + (direction.y * t)),
                (origin.z + (direction.z * t))};
    }
};

} // namespace AM
