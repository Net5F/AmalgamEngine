#pragma once

#include "ChunkPosition.h"

namespace AM
{
/**
 * A rectangle that encompasses a 2d range of chunks.
 */
struct ChunkRange {
public:
    /** The X-axis coordinate of the top left of the range. */
    int x{0};

    /** The Y-axis coordinate of the top left of the range. */
    int y{0};

    /** The X-axis length of this range. */
    int xLength{0};

    /** The Y-axis length of this range. */
    int yLength{0};

    /**
     * Sets this range to the union between itself and the given range.
     */
    void unionWith(const ChunkRange& other);

    /**
     * Sets this range to the intersection between itself and the given range.
     */
    void intersectWith(const ChunkRange& other);

    /** Returns true if the given position is within this range, else false.
        Note: This was made inline to follow what SDL does, without thinking
              any harder than that. Might be worth revisiting. */
    inline bool containsPosition(const ChunkPosition& position)
    {
        return ((position.x >= x) && (position.x < (x + xLength))
                && (position.y >= y) && (position.y < (y + yLength)));
    }
};

} // End namespace AM
