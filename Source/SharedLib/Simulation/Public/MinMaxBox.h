#pragma once

#include "Vector3.h"
#include "BoundingBox.h"

namespace AM
{

/**
 * A 3D axis-aligned box shape, using min and max points instead of center and 
 * half extent.
 *
 * A convenient alternative to BoundingBox, since we often need to work with 
 * min/max points.
 */
struct MinMaxBox {
public:
    /** This box's minimum point. */
    Vector3 min{};

    /** This box's maximum point. */
    Vector3 max{};

    MinMaxBox()
    : min{}
    , max{}
    {
    }

    MinMaxBox(const Vector3& inMin, const Vector3& inMax)
    : min{inMin}
    , max{inMax}
    {
    }

    explicit MinMaxBox(const BoundingBox& box)
    : min{(box.center.x - box.halfExtents.x),
          (box.center.y - box.halfExtents.y),
          (box.center.z - box.halfExtents.z)}
    , max{(box.center.x + box.halfExtents.x),
          (box.center.y + box.halfExtents.y),
          (box.center.z + box.halfExtents.z)}
    {
    }
};

} // End namespace AM
