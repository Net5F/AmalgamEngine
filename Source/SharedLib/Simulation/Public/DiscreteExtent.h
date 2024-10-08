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
    /** The X-axis coordinate of this extent's origin. */
    int x{0};

    /** The Y-axis coordinate of this extent's origin. */
    int y{0};

    /** The Z-axis coordinate of this extent's origin.
        Increasing Z is upwards. */
    int z{0};

    /** The X-axis length of this extent. */
    int xLength{0};

    /** The Y-axis length of this extent. */
    int yLength{0};

    /** The Z-axis length of this extent. */
    int zLength{0};

    DiscreteExtent()
    : x{0}
    , y{0}
    , z{0}
    , xLength{0}
    , yLength{0}
    , zLength{0}
    {
    }

    DiscreteExtent(int inX, int inY, int inZ, int inXLength, int inYLength,
                   int inZLength)
    : x{inX}
    , y{inY}
    , z{inZ}
    , xLength{inXLength}
    , yLength{inYLength}
    , zLength{inZLength}
    {
    }

    /**
     * Constructor that takes origin and extreme points to form a extent.
     */
    explicit DiscreteExtent(DiscretePosition<T> origin,
                            DiscretePosition<T> extreme)
    : x{origin.x}
    , y{origin.y}
    , z{origin.z}
    , xLength{extreme.x - origin.x}
    , yLength{extreme.y - origin.y}
    , zLength{extreme.z - origin.z}
    {
    }

    bool operator==(const DiscreteExtent<T>& other) const
    {
        return (x == other.x) && (y == other.y) && (z == other.z)
               && (xLength == other.xLength) && (yLength == other.yLength)
               && (zLength == other.zLength);
    }

    /**
     * Returns the max valid X position in this extent.
     * Note: Named differently from BoundingBox's 'maxX' member to avoid
     *       confusion (variable vs function).
     */
    int xMax() const { return (x + xLength - 1); }

    /**
     * Returns the max valid Y position in this extent.
     */
    int yMax() const { return (y + yLength - 1); }

    /**
     * Returns the max valid Z position in this extent.
     */
    int zMax() const { return (z + zLength - 1); }

    /**
     * Returns the minimum position of this extent.
     */
    DiscretePosition<T> min() const { return {x, y, z}; }

    /**
     * Returns the maximum position of this extent.
     */
    DiscretePosition<T> max() const { return {xMax(), yMax(), zMax()}; }

    /**
     * Returns the union between this extent and the given extent.
     */
    [[nodiscard]] DiscreteExtent<T>
        unionWith(const DiscreteExtent<T>& other) const
    {
        DiscreteExtent<T> finalExtent{*this};

        // Note: We can add some special fast cases for empty extents if we
        //       ever care to, but they likely wouldn't be exercised much.

        /* X union. */
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
        finalExtent.x = selfMin;
        finalExtent.xLength = (selfMax - selfMin);

        /* Y union. */
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

        finalExtent.y = selfMin;
        finalExtent.yLength = (selfMax - selfMin);

        /* Z union. */
        // Copy the logic from above, replacing X with Z.
        selfMin = z;
        selfMax = z + zLength;
        otherMin = other.z;
        otherMax = other.z + other.zLength;

        if (otherMin < selfMin) {
            selfMin = otherMin;
        }

        if (otherMax > selfMax) {
            selfMax = otherMax;
        }

        finalExtent.z = selfMin;
        finalExtent.zLength = (selfMax - selfMin);

        return finalExtent;
    }

    /**
     * Returns the intersection between this extent and the given extent.
     */
    [[nodiscard]] DiscreteExtent<T>
        intersectWith(const DiscreteExtent<T>& other) const
    {
        DiscreteExtent<T> finalExtent{*this};

        // Note: We can add some special fast cases for empty extents if we
        //       ever care to, but they likely wouldn't be exercised much.

        /* X intersection. */
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
        finalExtent.x = selfMin;
        finalExtent.xLength = (selfMax - selfMin);

        /* Y intersection. */
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

        finalExtent.y = selfMin;
        finalExtent.yLength = (selfMax - selfMin);

        /* Z intersection. */
        // Copy the logic from above, replacing X with Z.
        selfMin = z;
        selfMax = z + zLength;
        otherMin = other.z;
        otherMax = other.z + other.zLength;

        if (otherMin > selfMin) {
            selfMin = otherMin;
        }

        if (otherMax < selfMax) {
            selfMax = otherMax;
        }

        finalExtent.z = selfMin;
        finalExtent.zLength = (selfMax - selfMin);

        return finalExtent;
    }

    /**
     * @return true if the given position is within this extent, else false.
     */
    bool contains(const DiscretePosition<T>& position) const
    {
        return ((position.x >= x) && (position.x < (x + xLength))
                && (position.y >= y) && (position.y < (y + yLength))
                && (position.z >= z) && (position.z < (z + zLength)));
    }

    /**
     * @return true if the given extent is fully within this extent, else
     *         false.
     */
    bool contains(const DiscreteExtent<T>& extent) const
    {
        DiscretePosition<T> origin{extent.x, extent.y, extent.z};
        DiscretePosition<T> extreme{(extent.x + extent.xLength - 1),
                                    (extent.y + extent.yLength - 1),
                                    (extent.z + extent.zLength - 1)};
        return (contains(origin) && contains(extreme));
    }

    /**
     * @return true if this extent has no area.
     */
    bool isEmpty() const
    {
        return ((xLength <= 0) || (yLength <= 0) || (zLength <= 0));
    }

    /**
     * @return A count of the number of discrete elements within this extent.
     *         Can also be thought of as the volume of this extent.
     */
    std::size_t size() const { return (xLength * yLength * zLength); }
};

} // End namespace AM
