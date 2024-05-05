#pragma once

#include "DiscreteImpl.h"
#include <cmath>

namespace AM
{
/**
 * The position of a particular discrete unit (e.g. tiles, chunks).
 */
template<typename T>
struct DiscretePosition {
    // Note: Screens and maps start at (0, 0) so we could make these unsigned,
    //       but these are signed to facilitate using this struct for things
    //       like negative offsets.
    /** The X-axis coordinate of this position. */
    int x{0};

    /** The Y-axis coordinate of this position. */
    int y{0};

    bool operator==(const DiscretePosition<T>& other) const
    {
        return (x == other.x) && (y == other.y);
    }

    bool operator!=(const DiscretePosition<T>& other) const
    {
        return (x != other.x) || (y != other.y);
    }

    bool operator<(const DiscretePosition<T>& other) const
    {
        if (x < other.x)
            return true;
        if (x > other.x)
            return false;

        if (y < other.y)
            return true;
        if (y > other.y)
            return false;

        return false;
    }

    DiscretePosition<T> operator+(const DiscretePosition<T>& other) const
    {
        return {(x + other.x), (y + other.y)};
    }

    DiscretePosition<T>& operator+=(const DiscretePosition<T>& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    /**
     * If this position is within 1 unit of the given position, returns true.
     */
    bool isAdjacentTo(const DiscretePosition<T>& other) const
    {
        // Get the differences between the positions.
        int xDif{other.x - x};
        int yDif{other.y - y};

        // Square the differences.
        xDif *= xDif;
        yDif *= yDif;

        // If the absolute squared distance is within 1 unit, return true.
        return (std::abs(xDif + yDif) <= 1);
    }
};

} // End namespace AM
