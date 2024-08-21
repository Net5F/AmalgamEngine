#include "Ray.h"
#include "Log.h"

namespace AM
{

Vector3 Ray::getPointAtT(float t)
{
    return {(origin.x + (direction.x * t)), (origin.y + (direction.y * t)),
            (origin.z + (direction.z * t))};
}

void Ray::print() const
{
    LOG_INFO("Origin: (%.4f, %.4f, %.4f), Direction: (%.4f, %.4f, %.4f)",
             origin.x, origin.y, origin.z, direction.x, direction.y,
             direction.z);
}

} // End namespace AM
