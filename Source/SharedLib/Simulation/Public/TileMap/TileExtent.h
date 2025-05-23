#pragma once

#include "DiscreteExtent.h"

namespace AM
{
struct ChunkExtent;
struct CellExtent;
struct BoundingBox;
struct Cylinder;
struct Vector3;

/**
 * A strong type alias, describing an extent of map tiles.
 */
struct TileExtent : public DiscreteExtent<DiscreteImpl::TileTag> {
    TileExtent();

    TileExtent(int inX, int inY, int inZ, int inXLength, int inYLength,
               int inZLength);

    TileExtent(const DiscreteExtent<DiscreteImpl::TileTag>& tileExtent);

    explicit TileExtent(const ChunkExtent& chunkExtent);

    /**
     * @param cellWidthTiles The width of a cell, in tiles.
     * @param cellHeightTiles The height of a cell, in tiles.
     */
    explicit TileExtent(const CellExtent& cellExtent,
                        std::size_t cellWidthTiles,
                        std::size_t cellHeightTiles);

    /**
     * Constructs the smallest tile extent that contains the given bounding box.
     *
     * If the box is exactly on the edge of a tile, that tile will not be 
     * included in this extent.
     */
    explicit TileExtent(const BoundingBox& boundingBox);

    using DiscreteExtent<DiscreteImpl::TileTag>::contains;
    /**
     * @return true if the given box is fully within this extent, else
     *         false.
     */
    bool contains(const BoundingBox& box) const;

    /**
     * @return true if the given cylinder is fully within this extent, else
     *         false.
     */
    bool contains(const Cylinder& cylinder) const;

    /**
     * @return true if the given world point is within this extent.
     */
    bool contains(const Vector3& point) const;

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
