#pragma once

#include "TileExtent.h"
#include "Position.h"
#include "Ray.h"
#include "Log.h"
#include <cmath>

namespace AM
{
/**
 * Represents a set of axis-aligned 3D bounds, forming a box.
 */
struct BoundingBox {
public:
    float minX{0};
    float maxX{0};

    float minY{0};
    float maxY{0};

    float minZ{0};
    float maxZ{0};

    bool operator==(const BoundingBox& other);

    float getXLength() const;

    float getYLength() const;

    float getZLength() const;

    /**
     * Returns a position at the minimum point of this bounding box.
     */
    Position getMinPosition() const;

    /**
     * Returns a position at the maximum point of this bounding box.
     */
    Position getMaxPosition() const;

    /**
     * Returns a position at the center of this bounding box.
     *
     * Note: This center position is different than an entity's Position
     *       component. This one is centered in all axis, while an entity's
     *       Position is centered in the X/Y but is at the minimum Z.
     */
    Position get3dCenter() const;

    /**
     * Returns true if this box intersects the given other bounding box.
     */
    bool intersects(const BoundingBox& other) const;

    /**
     * Returns true if this box intersects the given cylinder.
     *
     * Note: This treats the given data as a cylinder with infinite length
     *       along the Z axis. If we want to treat it as a sphere, we can
     *       change it.
     * Note: Shared edges are considered to be intersecting.
     *
     * Reference: https://stackoverflow.com/a/402010/4258629
     */
    bool intersects(const Position& cylinderCenter, unsigned int radius) const;

    /**
     * Returns the t at which this box intersects the given ray.
     * Returns -1 if there's no intersection.
     */
    float intersects(const Ray& ray) const;

    /**
     * Returns true if this box intersects the given tile extent.
     *
     * Note: This treats the tile extent as having infinite length along the
     *       Z axis.
     * Note: Shared edges are considered to be intersecting.
     */
    bool intersects(const TileExtent& tileExtent) const;

    /**
     * Returns the smallest tile extent that contains this bounding box.
     *
     * Note: The Z-axis is ignored in this conversion, as TileExtent is 2D.
     */
    TileExtent asTileExtent() const;
};

} // End namespace AM
