#include "BoundingBox.h"
#include "Position.h"
#include "Cylinder.h"
#include "Ray.h"
#include "TileExtent.h"
#include "MovementHelpers.h"
#include "AMMath.h"
#include <cmath>

namespace AM
{

BoundingBox::BoundingBox()
: min{}
, max{}
{
}

bool BoundingBox::operator==(const BoundingBox& other) const
{
    return (min.x == other.min.x) && (max.x == other.max.x)
           && (min.y == other.min.y) && (max.y == other.max.y)
           && (min.z == other.min.z) && (max.z == other.max.z);
}

BoundingBox::BoundingBox(const TileExtent& tileExtent)
{
    static constexpr float TILE_WORLD_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_WORLD_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    min.x = tileExtent.x * TILE_WORLD_WIDTH;
    min.y = tileExtent.y * TILE_WORLD_WIDTH;
    min.z = tileExtent.z * TILE_WORLD_HEIGHT;

    max.x = (tileExtent.x + tileExtent.xLength) * TILE_WORLD_WIDTH;
    max.y = (tileExtent.y + tileExtent.yLength) * TILE_WORLD_WIDTH;
    max.z = (tileExtent.z + tileExtent.zLength) * TILE_WORLD_HEIGHT;
}

BoundingBox::BoundingBox(const Cylinder& cylinder)
: min{(cylinder.center.x - cylinder.radius),
      (cylinder.center.y - cylinder.radius),
      (cylinder.center.z - cylinder.radius)}
, max{(cylinder.center.x + cylinder.radius),
      (cylinder.center.y + cylinder.radius),
      (cylinder.center.z + cylinder.radius)}
{
}

float BoundingBox::xLength() const
{
    return (max.x - min.x);
}

float BoundingBox::yLength() const
{
    return (max.y - min.y);
}

float BoundingBox::zLength() const
{
    return (max.z - min.z);
}

Vector3 BoundingBox::getBottomCenterPoint() const
{
    return {(min.x + ((max.x - min.x) / 2.f)),
            (min.y + ((max.y - min.y) / 2.f)), min.z};
}

Vector3 BoundingBox::get3DCenterPoint() const
{
    return {(min.x + ((max.x - min.x) / 2.f)),
            (min.y + ((max.y - min.y) / 2.f)),
            (min.z + ((max.z - min.z) / 2.f))};
}

bool BoundingBox::isEmpty() const
{
    return ((min.x == max.x) || (min.y == max.y) || (min.z == max.z));
}

bool BoundingBox::intersects(const BoundingBox& other) const
{
    return ((min.x <= other.max.x) && (max.x >= other.min.x)
            && (min.y <= other.max.y) && (max.y >= other.min.y)
            && (min.z <= other.max.z) && (max.z >= other.min.z));
}

bool BoundingBox::intersects(const Cylinder& cylinder) const
{
    // Reference: https://stackoverflow.com/a/402010/4258629

    // If the cylinder doesn't intersect along the Z axis, return false.
    float cylinderMinZ{cylinder.center.z - cylinder.halfHeight};
    float cylinderMaxZ{cylinder.center.z + cylinder.halfHeight};
    if ((cylinderMaxZ < min.z) || (cylinderMinZ > max.z)) {
        return false;
    }

    // The cylinder intersects along the Z axis. The rest of the test now 
    // reduces to a 2D circle/rectangle intersection.

    // Get the X and Y distances between the centers.
    float centerX{min.x + ((max.x - min.x) / 2.f)};
    float centerY{min.y + ((max.y - min.y) / 2.f)};
    float circleDistanceX{std::abs(cylinder.center.x - centerX)};
    float circleDistanceY{std::abs(cylinder.center.y - centerY)};

    // If the circle is far enough away that no intersection is possible,
    // return false.
    float halfXLength{xLength() / 2.f};
    float halfYLength{yLength() / 2.f};
    if (circleDistanceX > (halfXLength + cylinder.radius)) {
        return false;
    }
    if (circleDistanceY > (halfYLength + cylinder.radius)) {
        return false;
    }

    // If the circle is close enough that an intersection is guaranteed,
    // return true.
    if (circleDistanceX <= halfXLength) {
        return true;
    }
    if (circleDistanceY <= halfYLength) {
        return true;
    }

    // Calculate the distance from the center of the circle to the corner
    // of the box.
    float xDif{circleDistanceX - halfXLength};
    float yDif{circleDistanceY - halfYLength};
    float cornerDistanceSquared{(xDif * xDif) + (yDif * yDif)};

    // If the distance is less than the radius, return true.
    return (cornerDistanceSquared <= (cylinder.radius * cylinder.radius));
}

bool BoundingBox::intersects(const TileExtent& tileExtent) const
{
    return intersects(BoundingBox(tileExtent));
}

bool BoundingBox::intersects(const Ray& ray) const
{
    Vector3 inverseRayDirection{ray.direction.reciprocal()};
    return intersects(ray.origin, inverseRayDirection, 0.f,
                      std::numeric_limits<float>::infinity())
        .didIntersect;
}

bool BoundingBox::intersects(const Vector3& start, const Vector3& end) const
{
    Vector3 inverseRayDirection{(end - start).reciprocal()};
    return intersects(start, inverseRayDirection, 0.f,
                      std::numeric_limits<float>::infinity())
        .didIntersect;
}

BoundingBox::RayIntersectReturn BoundingBox::intersects(const Ray& ray,
                                                        float tMinBound,
                                                        float tMaxBound) const
{
    Vector3 inverseRayDirection{ray.direction.reciprocal()};
    return intersects(ray.origin, inverseRayDirection, tMinBound, tMaxBound);
}

BoundingBox::RayIntersectReturn
    BoundingBox::intersects(const Vector3& rayOrigin,
                            const Vector3& inverseRayDirection, float tMinBound,
                            float tMaxBound) const
{
    // Slab Algorithm Ref: 
    // https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
    // https://technik90.blogspot.com/2018/06/the-other-pathtracer-4-optimizing-aabb.html

    Vector3 t0{(min - rayOrigin) * inverseRayDirection};
    Vector3 t1{(max - rayOrigin) * inverseRayDirection};
    
    Vector3 tEnter{Math::min(t0, t1)};
    Vector3 tExit{Math::max(t1, t0)};

    // We compare against tMinBound/tMaxBound so that, if the intersecting time
    // interval is not within [tMinBound, tMaxBound], either tMin will be 
    // brought above tMax, or tMax will be brought below tMin, causing the 
    // intersection check (tMax >= tMin) to return false.
    //
    // Example: Define tSlabMin = max(tEnter), tSlabMax = min(tExit).
    //          Let [tSlabMin, tSlabMax]   = [2, 4], 
    //              [tMinBound, tMaxBound] = [0, 1].
    //          Note that the result ([2, 4]) is outside of the bounds.
    //
    //          tLastEnter = max(tMin, tSlabMin) = max(0, 2) = 2
    //          tFirstExit = min(tMax, tSlabMax), = min(1, 4) = 1
    //
    //          We return (tFirstExit >= tLastEnter) -> return (1 >= 2) -> false.
    //
    // Note: There are also some cases where tMin/tMax get clamped to the 
    //       bounds, such as when rayOrigin is inside this box, or when the 
    //       bounds end before the exit is reached.
    float tMin{
        std::max(tMinBound, std::max(std::max(tEnter.x, tEnter.y), tEnter.z))};
    float tMax{
        std::min(tMaxBound, std::min(std::min(tExit.x, tExit.y), tExit.z))};

    return {(tMax >= tMin), tMin, tMax};
}

bool BoundingBox::contains(const BoundingBox& boundingBox) const
{
    return (min.x <= boundingBox.min.x) && (max.x >= boundingBox.max.x)
           && (min.y <= boundingBox.min.y) && (max.y >= boundingBox.max.y)
           && (min.z <= boundingBox.min.z) && (max.z >= boundingBox.max.z);
}

bool BoundingBox::contains(const Cylinder& cylinder) const
{
    BoundingBox cylinderWrapper(cylinder);
    return contains(cylinderWrapper);
}

bool BoundingBox::contains(const Vector3& worldPoint) const
{
    return (min.x <= worldPoint.x) && (max.x >= worldPoint.x)
           && (min.y <= worldPoint.y) && (max.y >= worldPoint.y)
           && (min.z <= worldPoint.z) && (max.z >= worldPoint.z);
}

BoundingBox BoundingBox::moveTo(const Vector3& newMin) const
{
    BoundingBox newBox{*this};
    Vector3 diff{newMin - newBox.min};
    newBox.min = newMin;
    newBox.max += diff;
    return newBox;
}

BoundingBox BoundingBox::moveBottomCenterTo(const Vector3& newBottomCenter) const
{
    BoundingBox newBox{*this};
    Vector3 lengths{xLength(), yLength(), zLength()};
    newBox.min.x = newBottomCenter.x - (lengths.x / 2.f);
    newBox.min.y = newBottomCenter.y - (lengths.y / 2.f);
    newBox.min.z = newBottomCenter.z;
    newBox.max = newBox.min + lengths;

    return newBox;
}

BoundingBox BoundingBox::translateBy(const Vector3& amountToMove) const
{
    BoundingBox newBox{*this};
    newBox.min += amountToMove;
    newBox.max += amountToMove;
    return newBox;
}

BoundingBox BoundingBox::unionWith(const BoundingBox& other) const
{
    BoundingBox finalBox{*this};

    finalBox.min.x = (min.x < other.min.x) ? min.x : other.min.x;
    finalBox.min.y = (min.y < other.min.y) ? min.y : other.min.y;
    finalBox.min.z = (min.z < other.min.z) ? min.z : other.min.z;

    finalBox.max.x = (max.x > other.max.x) ? max.x : other.max.x;
    finalBox.max.y = (max.y > other.max.y) ? max.y : other.max.y;
    finalBox.max.z = (max.z > other.max.z) ? max.z : other.max.z;

    return finalBox;
}

BoundingBox BoundingBox::expandBy(float amountToExpandBy) const
{
    Vector3 expandVector{amountToExpandBy, amountToExpandBy, amountToExpandBy};
    return {min - expandVector, max + expandVector};
}

void BoundingBox::print() const
{
    LOG_INFO("Min: (%.4f, %.4f, %.4f), Max: (%.4f, %.4f, %.4f)", min.x, min.y,
             min.z, max.x, max.y, max.z);
}

} // End namespace AM
