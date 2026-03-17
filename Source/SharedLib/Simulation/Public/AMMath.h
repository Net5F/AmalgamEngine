#pragma once

#include "Vector3.h"
#include "SharedConfig.h"

namespace AM
{
/**
 * STL-adjacent algorithms for our custom types.
 */
class Math
{
public:
    /** An epsilon that can be used when comparing float world positions to
        integer values, to account for float precision loss.
        Calculated by finding the float precision at the furthest position of
        the largest map that we support.
        Reference: https://blog.demofox.org/2017/11/21/floating-point-precision/
        Note: The division by 2 is because we center the map on the origin. */
    static constexpr float MAX_WORLD_VALUE{SharedConfig::MAX_MAP_WIDTH_TILES
                                           * SharedConfig::TILE_WORLD_WIDTH
                                           / 2.f};
    static constexpr float WORLD_EPSILON{MAX_WORLD_VALUE
                                         / static_cast<float>(1 << 23)};

    /** A generic value to use for "close enough" floating point comparisons. */
    static constexpr float GENERIC_EPSILON{1e-06f};

    /**
     * Returns a component-wise minimum of the given vectors.
     */
    static Vector3 min(const Vector3& a, const Vector3& b);

    /**
     * Returns a component-wise maximum of the given vectors.
     */
    static Vector3 max(const Vector3& a, const Vector3& b);

    /**
     * Returns a component-wise maximum of the given vectors.
     */
    static Vector3 abs(const Vector3& vector);

    /**
     * Returns true if the given values are approximately equal.
     * 
     * Uses WORLD_EPSILON as a tolerance threshold, making this useful for 
     * values that represent positions within the world map.
     */
    static bool isEqualApproxWorld(float a, float b);

    /**
     * Returns true if the given values are approximately equal.
     * 
     * Uses GENERIC_EPSILON as a tolerance threshold. If you're doing math on 
     * values the represent positions within the world map, consider using 
     * isEqualApproxWorld instead.
     */
    static bool isEqualApproxGeneric(float a, float b);
};

} // namespace AM
