#include "Cylinder.h"
#include "Position.h"
#include "BoundingBox.h"
#include <cmath>

namespace AM
{

bool Cylinder::intersects(const Position& position) const
{
    // Calc the X and Y components of the distance from our center to the point.
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
