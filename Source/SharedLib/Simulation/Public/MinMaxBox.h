#pragma once

#include "Vector3.h"

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
    
    void print();
};

} // End namespace AM
