#pragma once

#include "Position.h"

namespace AM
{
struct BoundingBox;

/**
 * Represents a cylinder.
 *
 * Note: We currently ignore the Z-axis, so this actually is effectively a 
 *       circle. If it's ever useful to incorporate a Z-axis length, we can do 
 *       so.
 */
struct Cylinder {
public:
    /** Center point. */
    Position center{};

    /** Radius in X/Y directions. */
    float radius{0};

    /**
     * Returns true if this cylinder intersects the given position.
     *
     * Note: Shared edges are considered to be intersecting.
     */
    bool intersects(const Position& position) const;

    /**
     * Returns true if this cylinder intersects the given bounding box.
     *
     * Note: Shared edges are considered to be intersecting.
     */
    bool intersects(const BoundingBox& boundingBox) const;
};

} // End namespace AM
