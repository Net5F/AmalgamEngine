#pragma once

#include "DiscreteExtent.h"
#include <SDL_stdinc.h>

namespace AM
{
struct TileExtent;

/**
 * A strong type alias, describing an extent of map chunks.
 */
struct ChunkExtent : public DiscreteExtent<DiscreteImpl::ChunkTag> {
    ChunkExtent();

    ChunkExtent(int inX, int inY, int inZ, int inXLength, int inYLength,
                int inZLength);

    explicit ChunkExtent(const TileExtent& tileExtent);

    /**
     * Prints this extent's current values.
     */
    void print() const;

    /**
     * Builds a ChunkExtent from the given tile map lengths, centering it 
     * around (0, 0) in the x and y directions, and starting at 0 in the z 
     * direction.
     */
    static ChunkExtent fromMapLengths(Uint16 mapXLengthChunks,
                                      Uint16 mapYLengthChunks,
                                      Uint16 mapZLengthChunks);
};

} // End namespace AM
