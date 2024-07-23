#include "Cylinder.h"
#include "Position.h"
#include "BoundingBox.h"
#include <cmath>

namespace AM
{

bool Cylinder::intersects(const Position& position) const
{
    // If the cylinder doesn't intersect along the Z axis, return false.
    float cylinderMinZ{center.z - halfHeight};
    float cylinderMaxZ{center.z + halfHeight};
    if ((position.z < cylinderMinZ) || (position.z > cylinderMaxZ)) {
        return false;
    }

    // The point intersects along the Z axis. The rest of the test now 
    // reduces to a 2D circle/rectangle intersection.

    // Calc the X/Y components of the distance from our center to the point.
    float distanceX{std::abs(center.x - position.x)};
    float distanceY{std::abs(center.y - position.y)};

    // Calc the distance. Keep it squared to avoid a sqrt.
    float distanceSquared{(distanceX * distanceX) + (distanceY * distanceY)};

    return (distanceSquared <= (radius * radius));
}

bool Cylinder::intersects(const BoundingBox& boundingBox) const
{
    return boundingBox.intersects(*this);
}

} // End namespace AM
