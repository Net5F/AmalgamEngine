#pragma once

#include "DiscreteExtent.h"

namespace AM
{

class ChunkExtent;

/**
 * A strong type alias, describing an extent of map tiles.
 */
class TileExtent : public DiscreteExtent<DiscreteImpl::TileTag>
{
public:
    TileExtent();

    TileExtent(int inX, int inY, int inXLength, int inYLength);

    explicit TileExtent(const ChunkExtent& chunkExtent);
};

} // End namespace AM
