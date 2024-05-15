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
    /** The X-axis coordinate of this position. */
    int x{0};

    /** The Y-axis coordinate of this position. */
    int y{0};

    /** The Z-axis coordinate of this position. */
    int z{0};

    bool operator==(const DiscretePosition<T>& other) const
    {
        return (x == other.x) && (y == other.y) && (z == other.z);
    }

    bool operator!=(const DiscretePosition<T>& other) const
    {
        return (x != other.x) || (y != other.y) || (z != other.z);
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

        if (z < other.z)
            return true;
        if (z > other.z)
            return false;

        return false;
    }

    DiscretePosition<T> operator+(const DiscretePosition<T>& other) const
    {
        return {(x + other.x), (y + other.y), (z + other.z)};
    }

    DiscretePosition<T>& operator+=(const DiscretePosition<T>& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    DiscretePosition<T> operator-(const DiscretePosition<T>& other) const
    {
        return {(x - other.x), (y - other.y), (z - other.z)};
    }

    DiscretePosition<T>& operator-=(const DiscretePosition<T>& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
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
        int zDif{other.z - z};

        // Square the differences.
        xDif *= xDif;
        yDif *= yDif;
        zDif *= zDif;

        // If the absolute squared distance is within 1 unit, return true.
        return (std::abs(xDif + yDif + zDif) <= 1);
    }
};

} // End namespace AM
