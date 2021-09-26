#include "ChunkRange.h"

namespace AM
{
void ChunkRange::setToUnion(const ChunkRange& other)
{
    // Note: We can add some special fast cases for empty ranges if we
    //       ever care to, but they likely wouldn't be exercised much.

    // If the other min x is lower, use it.
    if (other.x < x) {
        x = other.x;
    }

    // If the other max x is higher, use it.
    int selfMax = x + xLength;
    int otherMax = other.x + other.xLength;
    if (otherMax > selfMax) {
        xLength = (otherMax - x);
    }

    // If the other min y is lower, use it.
    if (other.y < y) {
        y = other.y;
    }

    // If the other max Y is higher, use it.
    selfMax = y + yLength;
    otherMax = other.y + other.yLength;
    if (otherMax > selfMax) {
        yLength = (otherMax - y);
    }
}

void ChunkRange::setToIntersect(const ChunkRange& other)
{
    // Note: We can add some special fast cases for empty ranges if we
    //       ever care to, but they likely wouldn't be exercised much.

    // If the other min x is higher, use it.
    if (other.x > x) {
        x = other.x;
    }

    // If the other max x is lower, use it.
    int selfMax = x + xLength;
    int otherMax = other.x + other.xLength;
    if (otherMax < selfMax) {
        xLength = (otherMax - x);
    }

    // If the other min y is higher, use it.
    if (other.y > y) {
        y = other.y;
    }

    // If the other max Y is lower, use it.
    selfMax = y + yLength;
    otherMax = other.y + other.yLength;
    if (otherMax < selfMax) {
        yLength = (otherMax - y);
    }
}

} // End namespace AM
