#pragma once

#include "Vector3.h"
#include <cmath>

namespace AM
{
/**
 * STL-adjacent algorithms for our custom types.
 */
class Math
{
public:

/**
 * Returns a component-wise minimum of the given vectors.
 */
static Vector3 min(const Vector3& a, const Vector3& b)
{
    Vector3 res;
    res.x = std::min(a.x, b.x);
    res.y = std::min(a.y, b.y);
    res.z = std::min(a.z, b.z);
    return res;
}

/**
 * Returns a component-wise maximum of the given vectors.
 */
static Vector3 max(const Vector3& a, const Vector3& b)
{
    Vector3 res;
    res.x = std::max(a.x, b.x);
    res.y = std::max(a.y, b.y);
    res.z = std::max(a.z, b.z);
    return res;
}

};

} // namespace AM
