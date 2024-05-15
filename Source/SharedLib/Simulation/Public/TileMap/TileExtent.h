#pragma once

#include "DiscreteExtent.h"

namespace AM
{
struct ChunkExtent;

/**
 * A strong type alias, describing an extent of map tiles.
 */
struct TileExtent : public DiscreteExtent<DiscreteImpl::TileTag> {
    TileExtent();

    TileExtent(int inX, int inY, int inZ, int inXLength, int inYLength,
               int inZLength);

    explicit TileExtent(const ChunkExtent& chunkExtent);
};

template<typename S>
void serialize(S& serializer, TileExtent& tileExtent)
{
    serializer.value4b(tileExtent.x);
    serializer.value4b(tileExtent.y);
    serializer.value4b(tileExtent.z);
    serializer.value4b(tileExtent.xLength);
    serializer.value4b(tileExtent.yLength);
    serializer.value4b(tileExtent.zLength);
}

} // End namespace AM
