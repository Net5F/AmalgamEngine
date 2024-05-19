#pragma once

#include <array>

namespace AM
{
struct Position;
struct Cylinder;
struct Ray;
struct TileExtent;

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
     * @return true if this box has no area.
     */
    bool isEmpty() const;

    /**
     * Returns true if this box intersects the given other bounding box.
     */
    bool intersects(const BoundingBox& other) const;

    /**
     * Returns true if this box intersects the given cylinder.
     *
     * Note: Shared edges are considered to be intersecting.
     */
    bool intersects(const Cylinder& cylinder) const;

    /**
     * Returns true if this box intersects the given ray.
     */
    bool intersects(const Ray& ray) const;

    /**
     * Returns true if this box intersects the given tile extent.
     *
     * Note: This treats the tile extent as having infinite length along the
     *       Z axis.
     * Note: Shared edges are considered to be intersecting.
     */
    bool intersects(const TileExtent& tileExtent) const;

    /**
     * Returns the minimum t at which this box intersects the given ray.
     * Returns -1 if there's no intersection.
     */
    float getMinIntersection(const Ray& ray) const;

    /**
     * Returns the maximum t at which this box intersects the given ray.
     * Returns -1 if there's no intersection.
     */
    float getMaxIntersection(const Ray& ray) const;

    /**
     * Returns the smallest tile extent that contains this bounding box.
     *
     * Note: The Z-axis is ignored in this conversion, as TileExtent is 2D.
     */
    TileExtent asTileExtent() const;

private:
    /**
     * Returns tMin and tMax for the given ray's intersection with this 
     * bounding box.
     * @return {tMin, tMax}
     */
    std::array<float, 2> getIntersections(const Ray& ray) const;
};

} // End namespace AM
