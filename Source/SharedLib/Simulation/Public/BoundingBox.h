#pragma once

#include "Vector3.h"
#include <array>

namespace AM
{
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
    /** This box's minimum point */
    Vector3 min;

    /** This box's maximum point */
    Vector3 max;

    BoundingBox();

    constexpr BoundingBox(const Vector3& inMin, const Vector3& inMax)
    : min{inMin}
    , max{inMax}
    {
    }

    BoundingBox(const TileExtent& tileExtent);

    bool operator==(const BoundingBox& other) const;

    float xLength() const;

    float yLength() const;

    float zLength() const;

    /**
     * Returns the point centered on this bounding box in the X and Y axis,
     * and at this box's minimum Z value.
     *
     * Note: This matches the placement of an entity's Position component, in 
     *       relation to the entity's bounding volume.
     */
    Vector3 getBottomCenterPoint() const;

    /**
     * Returns a position at the center of this bounding box.
     *
     * Note: This center position is different than an entity's Position
     *       component. This one is centered in all axis, while an entity's
     *       Position is centered in the X/Y but is at the minimum Z.
     */
    Vector3 get3DCenterPoint() const;

    /**
     * @return true if this box has no area.
     */
    bool isEmpty() const;

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
     * Returns true if this box contains the given world point.
     */
    bool contains(const Vector3& worldPoint) const;

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
     * Returns this box with its min point moved to the given point.
     */
    [[nodiscard]] BoundingBox moveTo(const Vector3& newMin) const;

    /**
     * Returns this box with its position translated by the given amount.
     */
    [[nodiscard]] BoundingBox translateBy(const Vector3& amountToMove) const;

    /**
     * Returns the union between this box and the given other box.
     */
    [[nodiscard]] BoundingBox unionWith(const BoundingBox& other) const;

    /**
     * Increases the size of this box by the given amount in all directions.
     */
    [[nodiscard]] BoundingBox expandBy(float amountToExpandBy) const;

    /**
     * Prints this box's current values.
     */
    void print() const;
};

} // End namespace AM
