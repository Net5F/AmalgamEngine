#include "BoundingBox.h"
#include "MinMaxBox.h"
#include "Position.h"
#include "Cylinder.h"
#include "Ray.h"
#include "TileExtent.h"
#include <cmath>

namespace AM
{

BoundingBox::BoundingBox()
: center{}
, halfExtents{}
{
}

BoundingBox::BoundingBox(const MinMaxBox& box)
: center{}
, halfExtents{}
{
    halfExtents.x = (box.max.x - box.min.x) / 2.f;
    halfExtents.y = (box.max.y - box.min.y) / 2.f;
    halfExtents.z = (box.max.z - box.min.z) / 2.f;
    center.x = box.min.x + halfExtents.x;
    center.y = box.min.y + halfExtents.y;
    center.z = box.min.z + halfExtents.z;
}

bool BoundingBox::operator==(const BoundingBox& other)
{
    return (center == other.center) && (halfExtents == other.halfExtents);
}

float BoundingBox::getXLength() const
{
    return (halfExtents.x * 2);
}

float BoundingBox::getYLength() const
{
    return (halfExtents.y * 2);
}

float BoundingBox::getZLength() const
{
    return (halfExtents.z * 2);
}

Vector3 BoundingBox::getMinPoint() const
{
    return {(center.x - halfExtents.x), (center.y - halfExtents.y),
            (center.z - halfExtents.z)};
}

Vector3 BoundingBox::getMaxPoint() const
{
    return {(center.x + halfExtents.x), (center.y + halfExtents.y),
            (center.z + halfExtents.z)};
}

Vector3 BoundingBox::getBottomCenterPoint() const
{
    return {center.x, center.y, (center.z - halfExtents.z)};
}

bool BoundingBox::isEmpty() const
{
    return (halfExtents.x == 0) || (halfExtents.y == 0) || (halfExtents.z == 0);
}

void BoundingBox::moveMinimumTo(const Vector3& point)
{
    center.x = point.x + halfExtents.x;
    center.y = point.y + halfExtents.y;
    center.z = point.z + halfExtents.z;
}

bool BoundingBox::intersects(const BoundingBox& other) const
{
    return ((std::abs(center.x - other.center.x)
             < (halfExtents.x + other.halfExtents.x))
            && (std::abs(center.y - other.center.y)
                < (halfExtents.y + other.halfExtents.y))
            && (std::abs(center.z - other.center.z)
                < (halfExtents.z + other.halfExtents.z)));
}

bool BoundingBox::intersects(const Cylinder& cylinder) const
{
    // Reference: https://stackoverflow.com/a/402010/4258629

    // TODO: Consider Z
    float xLength{getXLength()};
    float yLength{getYLength()};

    // Get the X and Y distances between the centers.
    float circleDistanceX{std::abs(cylinder.center.x - center.x)};
    float circleDistanceY{std::abs(cylinder.center.y - center.y)};

    // If the circle is far enough away that no intersection is possible,
    // return false.
    if (circleDistanceX > ((xLength / 2) + cylinder.radius)) {
        return false;
    }
    if (circleDistanceY > ((yLength / 2) + cylinder.radius)) {
        return false;
    }

    // If the circle is close enough that an intersection is guaranteed,
    // return true.
    if (circleDistanceX <= (xLength / 2)) {
        return true;
    }
    if (circleDistanceY <= (yLength / 2)) {
        return true;
    }

    // Calculate the distance from the center of the circle to the corner
    // of the box.
    float xDif{circleDistanceX - (xLength / 2)};
    float yDif{circleDistanceY - (yLength / 2)};
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
    static constexpr int TILE_WORLD_WIDTH{
        static_cast<int>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr int TILE_WORLD_HEIGHT{
        static_cast<int>(SharedConfig::TILE_WORLD_HEIGHT)};

    BoundingBox tileExtentBox{};
    tileExtentBox.halfExtents.x
        = ((tileExtent.xLength * TILE_WORLD_WIDTH) / 2.f);
    tileExtentBox.halfExtents.y
        = ((tileExtent.yLength * TILE_WORLD_WIDTH) / 2.f);
    tileExtentBox.halfExtents.z
        = ((tileExtent.zLength * TILE_WORLD_HEIGHT) / 2.f);
    tileExtentBox.center.x
        = (tileExtent.x * TILE_WORLD_WIDTH) + tileExtentBox.halfExtents.x;
    tileExtentBox.center.y
        = (tileExtent.y * TILE_WORLD_WIDTH) + tileExtentBox.halfExtents.y;
    tileExtentBox.center.z
        = (tileExtent.z * TILE_WORLD_HEIGHT) + tileExtentBox.halfExtents.z;

    return intersects(tileExtentBox);
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
    MinMaxBox box{*this};
    float tX1{(box.min.x - ray.origin.x) / ray.direction.x};
    float tX2{(box.max.x - ray.origin.x) / ray.direction.x};
    float tY1{(box.min.y - ray.origin.y) / ray.direction.y};
    float tY2{(box.max.y - ray.origin.y) / ray.direction.y};
    float tZ1{(box.min.z - ray.origin.z) / ray.direction.z};
    float tZ2{(box.max.z - ray.origin.z) / ray.direction.z};

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

TileExtent BoundingBox::asTileExtent() const
{
    static constexpr float TILE_WORLD_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_WORLD_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    TileExtent tileExtent{};
    Vector3 minPoint{getMinPoint()};
    tileExtent.x = static_cast<int>(std::floor(minPoint.x / TILE_WORLD_WIDTH));
    tileExtent.y = static_cast<int>(std::floor(minPoint.y / TILE_WORLD_WIDTH));
    tileExtent.z = static_cast<int>(std::floor(minPoint.z / TILE_WORLD_HEIGHT));
    tileExtent.xLength
        = static_cast<int>(std::ceil((halfExtents.x * 2.f) / TILE_WORLD_WIDTH));
    tileExtent.yLength
        = static_cast<int>(std::ceil((halfExtents.y * 2.f) / TILE_WORLD_WIDTH));
    tileExtent.zLength = static_cast<int>(
        std::ceil((halfExtents.z * 2.f) / TILE_WORLD_HEIGHT));

    return tileExtent;
}

} // End namespace AM
