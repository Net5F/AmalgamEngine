#include "MinMaxBox.h"
#include "BoundingBox.h"
#include "MovementHelpers.h"
#include "SharedConfig.h"
#include "Log.h"
#include <cmath>

namespace AM
{

MinMaxBox::MinMaxBox()
: min{}
, max{}
{
}

MinMaxBox::MinMaxBox(const Vector3& inMin, const Vector3& inMax)
: min{inMin}
, max{inMax}
{
}

MinMaxBox::MinMaxBox(const BoundingBox& box)
: min{(box.center.x - box.halfExtents.x),
      (box.center.y - box.halfExtents.y),
      (box.center.z - box.halfExtents.z)}
, max{(box.center.x + box.halfExtents.x),
      (box.center.y + box.halfExtents.y),
      (box.center.z + box.halfExtents.z)}
{
}

void MinMaxBox::print()
{
    LOG_INFO("Min: (%.4f, %.4f, %.4f), Max: (%.4f, %.4f, %.4f)", min.x,
             min.y, min.z, max.x, max.y, max.z);
}

} // End namespace AM
