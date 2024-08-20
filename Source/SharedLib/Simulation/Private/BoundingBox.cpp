#include "BoundingBox.h"
#include "Position.h"
#include "Cylinder.h"
#include "Ray.h"
#include "TileExtent.h"
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

float BoundingBox::getXLength() const
{
    return (max.x - min.x);
}

float BoundingBox::getYLength() const
{
    return (max.y - min.y);
}

float BoundingBox::getZLength() const
{
    return (max.z - min.z);
}

Vector3 BoundingBox::getBottomCenterPoint() const
{
    return {min.x + ((max.x - min.x) / 2.f), min.y + ((max.y - min.y) / 2.f),
            min.z};
}

Vector3 BoundingBox::get3DCenterPoint() const
{
    return {min.x + ((max.x - min.x) / 2.f), min.y + ((max.y - min.y) / 2.f),
            min.z + ((max.z - min.z) / 2.f)};
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
    float halfXLength{getXLength() / 2.f};
    float halfYLength{getYLength() / 2.f};
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

bool BoundingBox::intersects(const Ray& ray) const
{
    auto [tMin, tMax] = getIntersections(ray);

    // If tMax is negative, the ray would have to go in the negative direction
    // to intersect the rect.
    // If tMin > tMax, no intersection.
    if ((tMax < 0) || (tMin > tMax)) {
        return false;
    }

    return true;
}

bool BoundingBox::intersects(const TileExtent& tileExtent) const
{
    return intersects(BoundingBox(tileExtent));
}

bool BoundingBox::contains(const BoundingBox& boundingBox) const
{
    return (min.x <= boundingBox.min.x) && (max.x >= boundingBox.max.x)
           && (min.y <= boundingBox.min.y) && (max.y >= boundingBox.max.y)
           && (min.z <= boundingBox.min.z) && (max.z >= boundingBox.max.z);
}

bool BoundingBox::contains(const Vector3& worldPoint) const
{
    return (min.x <= worldPoint.x) && (max.x >= worldPoint.x)
           && (min.y <= worldPoint.y) && (max.y >= worldPoint.y)
           && (min.z <= worldPoint.z) && (max.z >= worldPoint.z);
}

float BoundingBox::getMinIntersection(const Ray& ray) const
{
    auto [tMin, tMax] = getIntersections(ray);

    // If tMax is negative, the ray would have to go in the negative direction
    // to intersect the rect.
    // If tMin > tMax, no intersection.
    if ((tMax < 0) || (tMin > tMax)) {
        return -1;
    }

    // If tMin doesn't intersect in the forward direction, return tMax.
    if (tMin < 0) {
        return tMax;
    }

    return tMin;
}

float BoundingBox::getMaxIntersection(const Ray& ray) const
{
    auto [tMin, tMax] = getIntersections(ray);

    // If tMax is negative, the ray would have to go in the negative direction
    // to intersect the rect.
    // If tMin > tMax, no intersection.
    if ((tMax < 0) || (tMin > tMax)) {
        return -1;
    }

    return tMax;
}

std::array<float, 2> BoundingBox::getIntersections(const Ray& ray) const
{
    // Find the constant t where intersection occurs for each direction.
    float tX1{(min.x - ray.origin.x) / ray.direction.x};
    float tX2{(max.x - ray.origin.x) / ray.direction.x};
    float tY1{(min.y - ray.origin.y) / ray.direction.y};
    float tY2{(max.y - ray.origin.y) / ray.direction.y};
    float tZ1{(min.z - ray.origin.z) / ray.direction.z};
    float tZ2{(max.z - ray.origin.z) / ray.direction.z};

    // Find the min t in each direction, then find the max of those.
    // This gives us the t where the ray first intersects the rect.
    float tMin{std::max(std::max(std::min(tX1, tX2), std::min(tY1, tY2)),
                        std::min(tZ1, tZ2))};

    // Find the max t in each direction, then find the min of those.
    // This gives us the t where the ray last intersects the rect.
    float tMax{std::min(std::min(std::max(tX1, tX2), std::max(tY1, tY2)),
                        std::max(tZ1, tZ2))};

    return {tMin, tMax};
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
