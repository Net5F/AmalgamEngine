#pragma once

#include "Vector3.h"
#include "TileExtent.h"
#include "Log.h"

namespace AM
{
struct BoundingBox;

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

    MinMaxBox();

    MinMaxBox(const Vector3& inMin, const Vector3& inMax);

    explicit MinMaxBox(const BoundingBox& box);

    /**
     * Returns the smallest tile extent that contains this bounding box.
     */
    TileExtent asTileExtent() const;
    
    void print()
    {
        LOG_INFO("Min: (%.4f, %.4f, %.4f), Max: (%.4f, %.4f, %.4f)", min.x,
                 min.y, min.z, max.x, max.y, max.z);
    }
};

} // End namespace AM
