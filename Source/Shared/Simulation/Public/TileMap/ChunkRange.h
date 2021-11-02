#pragma once

#include "DiscreteRangeTools.h"
#include "ChunkPosition.h"

namespace AM
{
/**
 * A rectangle that encompasses a range of map chunks.
 *
 * Note: Ideally this would be a strong alias of a DiscreteRange class, but
 *       there isn't a way to accomplish that without making the interface
 *       more clumsy, or losing our POD-type status.
 *       Instead, we've factored the hard-to-maintain logic out into a set of
 *       free functions that act on an isDiscreteRange concept.
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
    void unionWith(const ChunkRange& other) {
        DiscreteRangeTools::unionRange(*this, other);
    }

    /**
     * Sets this range to the intersection between itself and the given range.
     */
    void intersectWith(const ChunkRange& other){
        DiscreteRangeTools::intersectRange(*this, other);
    }

    /** Returns true if the given position is within this range, else false. */
    inline bool containsPosition(const ChunkPosition& position)
    {
        return DiscreteRangeTools::containsPosition(*this, position);
    }
};

} // End namespace AM
