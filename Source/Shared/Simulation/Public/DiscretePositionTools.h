#pragma once

#include <concepts>
#include <type_traits>
#include <cmath>

namespace AM
{

/**
 * Utility functions that act on discrete positions (ChunkPosition,
 * TilePosition).
 */
namespace DiscretePositionTools
{
/**
 * Tests if T has members x and y of integral type.
 *
 * Note: This is currently 2D to match our map, but may eventually become 3D.
 */
template <typename T>
concept isDiscretePosition = std::conjunction_v<
  std::is_integral<decltype(T::x)>,
  std::is_integral<decltype(T::y)>
>;

/**
 * @return true if base is within 1 unit of other.
 */
template <typename T>
requires isDiscretePosition<T>
static bool isAdjacent(const T& base, const T& other)
{
    // Get the differences between the positions.
    int xDif{other.x - base.x};
    int yDif{other.y - base.y};

    // Square the differences.
    xDif *= xDif;
    yDif *= yDif;

    // Use the absolute squared distance to check if we're adjacent.
    // Note: This relies on 1^2 == 1.
    return (std::abs(xDif + yDif) <= 1);
}
};

} // End namespace AM
