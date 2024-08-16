#pragma once

#include "Vector3.h"
#include <array>

namespace AM
{
struct MinMaxBox;
struct Position;
struct Cylinder;
struct Ray;
struct TileExtent;
struct TilePosition;

/**
 * A 3D axis-aligned box shape.
 */
struct BoundingBox {
public:
    /** This box's center point. */
    Vector3 center{};

    /** This box's half extents in each direction. */
    Vector3 halfExtents{};

    BoundingBox();

    constexpr BoundingBox(const Vector3& inCenter, const Vector3& inHalfExtents)
    : center{inCenter}
    , halfExtents{inHalfExtents}
    {
    }

    explicit BoundingBox(const MinMaxBox& box);

    bool operator==(const BoundingBox& other) const;

    float getXLength() const;

    float getYLength() const;

    float getZLength() const;

    /**
     * Returns the minimum point of this bounding box.
     */
    Vector3 min() const;

    /**
     * Returns the maximum point of this bounding box.
     */
    Vector3 max() const;

    /**
     * Returns the point centered on this bounding box in the X and Y axis,
     * and at this box's minimum Z value.
     *
     * This matches the placement of an entity's Position component, in 
     * relation to the entity's bounding volume.
     */
    Vector3 getBottomCenterPoint() const;

    /**
     * @return true if this box has no area.
     */
    bool isEmpty() const;

    /**
     * Translates this bounding box so that its minimum point is at the given 
     * point, but its extent remains unchanged.
     */
    void moveMinimumTo(const Vector3& point);

    /**
     * Translates this bounding box so that its bottom center point is at the 
     * given entity position, but its extent remains unchanged.
     */
    void moveToEntityPosition(const Position& position);

    /**
     * Returns true if this box intersects the given other bounding box.
     * Note: Shared edges are considered to be intersecting.
     */
    bool intersects(const BoundingBox& other) const;

    /**
     * Returns true if this box intersects the given cylinder.
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
     * Returns true if this box fully encloses the given other bounding box.
     * Note: Shared edges are considered to be contained.
     */
    bool contains(const BoundingBox& boundingBox) const;

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
     * Returns tMin and tMax for the given ray's intersection with this 
     * bounding box.
     * @return {tMin, tMax}
     */
    std::array<float, 2> getIntersections(const Ray& ray) const;

    /**
     * Sets this box to the union between itself and the given box.
     */
    void unionWith(const BoundingBox& other);

    /**
     * Returns the amount of overlap between this bounding box and the given 
     * bounding box along each axis.
     */
    Vector3 getOverlap(const BoundingBox& other) const;

    /**
     * Prints this box's current values.
     */
    void print() const;
};

} // End namespace AM
