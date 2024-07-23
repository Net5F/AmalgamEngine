#pragma once

#include "Position.h"

namespace AM
{
struct BoundingBox;

/**
 * Represents a capped cylinder.
 */
struct Cylinder {
    /** Center point. */
    Position center{};

    /** Radius along the X/Y axes. */
    float radius{0};

    /** Half height along the Z axis. */
    float halfHeight{0};

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
