#pragma once

#include "DiscreteExtent.h"

namespace AM
{
struct ChunkExtent;
struct BoundingBox;
struct MinMaxBox;

/**
 * A strong type alias, describing an extent of map tiles.
 */
struct TileExtent : public DiscreteExtent<DiscreteImpl::TileTag> {
    TileExtent();

    TileExtent(int inX, int inY, int inZ, int inXLength, int inYLength,
               int inZLength);

    explicit TileExtent(const ChunkExtent& chunkExtent);

    /**
     * Constructs the smallest tile extent that contains the given bounding box.
     *
     * If the box is exactly on the edge of a tile, that tile will not be 
     * included in this extent.
     */
    explicit TileExtent(const BoundingBox& boundingBox);

    /**
     * Constructs the smallest tile extent that contains the given min max box.
     *
     * If the box is exactly on the edge of a tile, that tile will not be 
     * included in this extent.
     */
    explicit TileExtent(const MinMaxBox& box);

    /**
     * Prints this extent's current values.
     */
    void print() const;
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
