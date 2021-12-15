#pragma once

#include "DiscreteTagTypes.h"
#include "DiscretePosition.h"

namespace AM
{

/**
 * A rectangle that encloses discretely-positioned things (e.g. tiles, chunks).
 */
template <typename T>
struct DiscreteExtent {
public:
    // Note: Screens and maps start at (0, 0) so we could make these unsigned,
    //       but these are signed to facilitate using this struct for things
    //       like negative offsets.
    DiscreteExtent()
    : x{0}
    , y{0}
    , xLength{0}
    , yLength{0}
    {
    }

    DiscreteExtent(int inX, int inY, int inXLength, int inYLength)
    : x{inX}
    , y{inY}
    , xLength{inXLength}
    , yLength{inYLength}
    {
    }

    /**
     * Constructor that takes a top left and bottom right point to form a range.
     */
    DiscreteExtent(DiscretePosition<T> topLeft, DiscretePosition<T> bottomRight)
    : x{topLeft.x}
    , y{topLeft.y}
    , xLength{bottomRight.x - topLeft.x}
    , yLength{bottomRight.y - topLeft.y}
    {
    }

    /** The X-axis coordinate of the top left of the range. */
    int x{0};

    /** The Y-axis coordinate of the top left of the range. */
    int y{0};

    /** The X-axis length of this range. */
    int xLength{0};

    /** The Y-axis length of this range. */
    int yLength{0};

    /**
     * Returns the max X position in this range.
     * Kept as a function instead of a variable for brevity of initialization.
     */
    int xMax() {
        return (x + xLength);
    }

    /**
     * Returns the max Y position in this range.
     * Kept as a function instead of a variable for brevity of initialization.
     */
    int yMax() {
        return (y + yLength);
    }

    /**
     * Sets this range to the union between itself and the given range.
     */
    void unionWith(const DiscreteExtent<T>& other) {
        // Note: We can add some special fast cases for empty ranges if we
        //       ever care to, but they likely wouldn't be exercised much.

        /* Horizontal union. */
        // Calc the min and max X coordinates for both ranges.
        int selfMin{x};
        int selfMax{x + xLength};
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
        x = selfMin;
        xLength = (selfMax - selfMin);

        /* Vertical union. */
        // Copy the logic from above, replacing X with Y.

        selfMin = y;
        selfMax = y + yLength;
        otherMin = other.y;
        otherMax = other.y + other.yLength;

        if (otherMin < selfMin) {
            selfMin = otherMin;
        }

        if (otherMax > selfMax) {
            selfMax = otherMax;
        }

        y = selfMin;
        yLength = (selfMax - selfMin);
    }

    /**
     * Sets this range to the intersection between itself and the given range.
     */
    void intersectWith(const DiscreteExtent<T>& other){
        // Note: We can add some special fast cases for empty ranges if we
        //       ever care to, but they likely wouldn't be exercised much.

        /* Horizontal intersection. */
        // Calc the min and max X coordinates for both ranges.
        int selfMin{x};
        int selfMax{x + xLength};
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
        x = selfMin;
        xLength = (selfMax - selfMin);

        /* Vertical intersection. */
        // Copy the logic from above, replacing X with Y.
        selfMin = y;
        selfMax = y + yLength;
        otherMin = other.y;
        otherMax = other.y + other.yLength;

        if (otherMin > selfMin) {
            selfMin = otherMin;
        }

        if (otherMax < selfMax) {
            selfMax = otherMax;
        }

        y = selfMin;
        yLength = (selfMax - selfMin);
    }

    /**
     * Returns true if the given position is within this range, else false.
     */
    bool containsPosition(const DiscretePosition<T>& position) const
    {
        return ((position.x >= x) && (position.x < (x + xLength))
                && (position.y >= y) && (position.y < (y + yLength)));
    }

    /**
     * Returns the number of things in this extent.
     */
    std::size_t getCount() const
    {
        return (xLength * yLength);
    }
};

} // End namespace AM
