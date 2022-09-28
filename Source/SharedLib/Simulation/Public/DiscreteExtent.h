#pragma once

#include "DiscreteImpl.h"
#include "DiscretePosition.h"

namespace AM
{
/**
 * A rectangle that encloses discretely-positioned things (e.g. tiles, chunks).
 */
template<typename T>
struct DiscreteExtent {
public:
    // Note: Screens and maps start at (0, 0) so we could make these unsigned,
    //       but these are signed to facilitate using this struct for things
    //       like negative offsets.
    /** The X-axis coordinate of the top left of the extent. */
    int x{0};

    /** The Y-axis coordinate of the top left of the extent. */
    int y{0};

    /** The X-axis length of this extent. */
    int xLength{0};

    /** The Y-axis length of this extent. */
    int yLength{0};

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
     * Constructor that takes a top left and bottom right point to form a
     * extent.
     */
    explicit DiscreteExtent(DiscretePosition<T> topLeft,
                            DiscretePosition<T> bottomRight)
    : x{topLeft.x}
    , y{topLeft.y}
    , xLength{bottomRight.x - topLeft.x}
    , yLength{bottomRight.y - topLeft.y}
    {
    }

    /**
     * Returns the max X position in this extent.
     */
    int xMax() const { return (x + xLength); }

    /**
     * Returns the max Y position in this extent.
     */
    int yMax() const { return (y + yLength); }

    /**
     * Sets this extent to the union between itself and the given extent.
     */
    void unionWith(const DiscreteExtent<T>& other)
    {
        // Note: We can add some special fast cases for empty extents if we
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
     * Sets this extent to the intersection between itself and the given extent.
     */
    void intersectWith(const DiscreteExtent<T>& other)
    {
        // Note: We can add some special fast cases for empty extents if we
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
     * @return true if the given position is within this extent, else false.
     */
    bool containsPosition(const DiscretePosition<T>& position) const
    {
        return ((position.x >= x) && (position.x < (x + xLength))
                && (position.y >= y) && (position.y < (y + yLength)));
    }

    /**
     * @return true if the given extent is fully within this extent, else
     *         false.
     */
    bool containsExtent(const DiscreteExtent<T>& extent) const
    {
        DiscretePosition<T> topLeft{extent.x, extent.y};
        DiscretePosition<T> bottomRight{(extent.x + extent.xLength - 1),
                                        (extent.y + extent.yLength - 1)};
        return (containsPosition(topLeft) && containsPosition(bottomRight));
    }

    /**
     * @return true if this extent has no area.
     */
    bool isEmpty() { return ((xLength <= 0) || (yLength <= 0)); }

    /**
     * @return A count of the number of discrete elements within this extent.
     *         Can also be thought of as the area of this extent.
     */
    std::size_t getCount() const { return (xLength * yLength); }
};

} // End namespace AM
