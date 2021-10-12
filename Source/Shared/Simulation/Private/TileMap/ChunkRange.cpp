#include "ChunkRange.h"
#include "Log.h"

namespace AM
{
void ChunkRange::unionWith(const ChunkRange& other)
{
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

void ChunkRange::intersectWith(const ChunkRange& other)
{
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

} // End namespace AM
