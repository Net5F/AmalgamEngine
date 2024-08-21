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
     * Returns the point along this ray at the given t.
     */
    Vector3 getPointAtT(float t);

    /**
     * Prints this ray's current values.
     */
    void print() const;
};

} // namespace AM
