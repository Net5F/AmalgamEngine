#pragma once

#include "DiscreteExtent.h"

namespace AM
{
class TileExtent;

/**
 * A strong type alias, describing an extent of map chunks.
 */
class ChunkExtent : public DiscreteExtent<DiscreteImpl::ChunkTag>
{
public:
    ChunkExtent();

    ChunkExtent(int inX, int inY, int inXLength, int inYLength);

    explicit ChunkExtent(const TileExtent& tileExtent);
};

} // End namespace AM
