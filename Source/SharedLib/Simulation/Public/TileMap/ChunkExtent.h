#pragma once

#include "DiscreteExtent.h"

namespace AM
{
struct TileExtent;

/**
 * A strong type alias, describing an extent of map chunks.
 */
struct ChunkExtent : public DiscreteExtent<DiscreteImpl::ChunkTag> {
    ChunkExtent();

    ChunkExtent(int inX, int inY, int inXLength, int inYLength);

    explicit ChunkExtent(const TileExtent& tileExtent);
};

} // End namespace AM
