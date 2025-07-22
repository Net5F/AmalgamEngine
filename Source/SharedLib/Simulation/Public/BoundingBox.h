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

    /**
     * Constructs a box that tightly wraps the given cylinder.
     */
    BoundingBox(const Cylinder& cylinder);

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
     * Returns true if this box intersects the given tile extent.
     *
     * Note: This treats the tile extent as having infinite length along the
     *       Z axis.
     * Note: Shared edges are considered to be intersecting.
     */
    bool intersects(const TileExtent& tileExtent) const;

    /**
     * Returns true if this box intersects the given ray.
     */
    bool intersects(const Ray& ray) const;

    /**
     * Returns true if this box intersects the given line.
     */
    bool intersects(const Vector3& start, const Vector3& end) const;

    struct RayIntersectReturn
    {
        /** If true, an intersection occurred. */
        bool didIntersect{};
        /** If didIntersect == true, this is the t value where the ray first 
            intersects the box.
            If the ray origin is inside the box, this will be <= 0 and
            clamped to tMinBound. */
        float tMin{};
        /** If didIntersect == true, this is the t value where the ray last
            intersects the box.
            This will be clamped to tMaxBound. */
        float tMax{};
    };
    /**
     * Looks for an intersection between this box and the given ray, within the 
     * range [tMinBound, tMaxBound].
     *
     * Note: If you're going to constrain to a magnitude-independent range like
     *       [0, 1], remember to normalize the ray's direction before taking the 
     *       reciprocal. However, if the magnitude is important to your 
     *       calculations (e.g. if you want to compare the resultant tMin to a
     *       distance), don't normalize.
     */
    RayIntersectReturn intersects(const Ray& ray, float tMinBound,
                                  float tMaxBound) const;

    /**
     * Looks for an intersection between this box and the given ray, within the 
     * range [tMinBound, tMaxBound].
     *
     * Note: See intersects(Ray) for a note on normalization.
     */
    RayIntersectReturn intersects(const Vector3& rayOrigin,
                                  const Vector3& inverseRayDirection,
                                  float tMinBound, float tMaxBound) const;

    /**
     * Returns true if this box fully encloses the given other bounding box.
     * Note: Shared edges are considered to be contained.
     */
    bool contains(const BoundingBox& boundingBox) const;

    /**
     * Returns true if this box fully encloses the given cylinder.
     * Note: Shared edges are considered to be contained.
     */
    bool contains(const Cylinder& cylinder) const;

    /**
     * Returns true if this box contains the given world point.
     */
    bool contains(const Vector3& worldPoint) const;

    /**
     * Returns this box with its min point moved to the given point.
     */
    [[nodiscard]] BoundingBox moveTo(const Vector3& newMin) const;

    /**
     * Returns this box with its bottom center moved to the given point 
     * (centered along X/Y axis, min.z == newBottomCenter.z).
     *
     * Note: This matches the placement of an entity's Position component, in 
     *       relation to the entity's bounding volume.
     */
    [[nodiscard]] BoundingBox
        moveBottomCenterTo(const Vector3& newBottomCenter) const;

    /**
     * Returns this box with its position translated by the given amount.
     */
    [[nodiscard]] BoundingBox translateBy(const Vector3& amountToMove) const;

    /**
     * Returns the union between this box and the given other box.
     */
    [[nodiscard]] BoundingBox unionWith(const BoundingBox& other) const;

    /**
     * Returns this box with its size increased by the given amount in all 
     * directions.
     */
    [[nodiscard]] BoundingBox expandBy(float amountToExpandBy) const;

    /**
     * Prints this box's current values.
     */
    void print() const;
};

} // End namespace AM
