#pragma once

#include "DiscretePositionTools.h"
#include <concepts>
#include <type_traits>

namespace AM
{

/**
 * Utility functions that act on discrete ranges (ChunkRange, TileRange).
 */
namespace DiscreteRangeTools
{
/**
 * Tests if T has members x, y, xLength, and yLength of integral type.
 *
 * Note: This is currently 2D to match our map, but may eventually become 3D.
 */
template <typename T>
concept isDiscreteRange = std::conjunction_v<
  std::is_integral<decltype(T::x)>,
  std::is_integral<decltype(T::y)>,
  std::is_integral<decltype(T::xLength)>,
  std::is_integral<decltype(T::yLength)>
>;

/**
 * Sets base to the union between itself and other.
 */
template <typename T>
requires isDiscreteRange<T>
static void unionRange(T& base, const T& other)
{
    // Note: We can add some special fast cases for empty ranges if we
    //       ever care to, but they likely wouldn't be exercised much.

    /* Horizontal union. */
    // Calc the min and max X coordinates for both ranges.
    int selfMin{base.x};
    int selfMax{base.x + base.xLength};
    int otherMin{other.x};
    int otherMax{other.x + other.xLength};

    // Determine which min and max are the least constrained.
    if (otherMin < selfMin) {
        selfMin = otherMin;
    }

    if (otherMax > selfMax) {
        selfMax = otherMax;
    }

    // Save the new min and length.
    base.x = selfMin;
    base.xLength = (selfMax - selfMin);

    /* Vertical union. */
    // Copy the logic from above, replacing X with Y.

    selfMin = base.y;
    selfMax = base.y + base.yLength;
    otherMin = other.y;
    otherMax = other.y + other.yLength;

    if (otherMin < selfMin) {
        selfMin = otherMin;
    }

    if (otherMax > selfMax) {
        selfMax = otherMax;
    }

    base.y = selfMin;
    base.yLength = (selfMax - selfMin);
}

/**
 * Sets base to the intersection between itself and other.
 */
template <typename T>
requires isDiscreteRange<T>
static void intersectRange(T& base, const T& other)
{
    // Note: We can add some special fast cases for empty ranges if we
    //       ever care to, but they likely wouldn't be exercised much.

    /* Horizontal intersection. */
    // Calc the min and max X coordinates for both ranges.
    int selfMin{base.x};
    int selfMax{base.x + base.xLength};
    int otherMin{other.x};
    int otherMax{other.x + other.xLength};

    // Determine which min and max are the most constrained.
    if (otherMin > selfMin) {
        selfMin = otherMin;
    }

    if (otherMax < selfMax) {
        selfMax = otherMax;
    }

    // Save the new min and length.
    base.x = selfMin;
    base.xLength = (selfMax - selfMin);

    /* Vertical intersection. */
    // Copy the logic from above, replacing X with Y.

    selfMin = base.y;
    selfMax = base.y + base.yLength;
    otherMin = other.y;
    otherMax = other.y + other.yLength;

    if (otherMin > selfMin) {
        selfMin = otherMin;
    }

    if (otherMax < selfMax) {
        selfMax = otherMax;
    }

    base.y = selfMin;
    base.yLength = (selfMax - selfMin);
}

/** Returns true if the given position is within this range, else false. */
template <typename TRange, typename TPosition>
requires isDiscreteRange<TRange> && DiscretePositionTools::isDiscretePosition<TPosition>
static bool containsPosition(const TRange& range, const TPosition& position)
{
    return ((position.x >= range.x) && (position.x < (range.x + range.xLength))
            && (position.y >= range.y) && (position.y < (range.y + range.yLength)));
}
};

} // End namespace AM
