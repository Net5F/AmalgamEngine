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

    /**
     * Linearly maps x in range [a, b] to an equivalent value in range [0, 1].
     *
     * If x is outside [a, b], the returned value will be outside [0, 1].
     * For example, x < a returns a value less than 0, and x > b returns a value
     * greater than 1.
     */
    static float inverseLerp(float a, float b, float x);

    /**
     * Smoothly maps x in range [edge0, edge1] to an equivalent value in range
     * [0, 1].
     * 
     * Values outside [edge0, edge1] are clamped to [0, 1].
     *
     * Uses a smooth curve with shallow slope near the edges and a steeper 
     * slope through the middle.
     *
     * Note: This normally steps from smaller values to larger values. To step
     *       from larger values to smaller values, use:
     *         1.0f - smoothstep(edge0, edge1, x)
     */
    static float smoothstep(float edge0, float edge1, float x);
};

} // namespace AM
