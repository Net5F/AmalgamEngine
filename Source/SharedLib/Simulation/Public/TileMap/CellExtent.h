#pragma once

#include "DiscreteExtent.h"

namespace AM
{
struct TileExtent;
struct BoundingBox;
struct MinMaxBox;

/**
 * A strong type alias, describing an extent of spatial partitioning grid
 * cells.
 */
struct CellExtent : public DiscreteExtent<DiscreteImpl::CellTag> {
    CellExtent();

    CellExtent(int inX, int inY, int inZ, int inXLength, int inYLength,
               int inZLength);

    /**
     * Constructs the smallest cell extent that contains the given tile extent.
     *
     * @param cellWidthTiles The width of a cell, in tiles.
     * @param cellHeightTiles The height of a cell, in tiles.
     */
    explicit CellExtent(const TileExtent& tileExtent,
                        std::size_t cellWidthTiles,
                        std::size_t cellHeightTiles);

    /**
     * Constructs the smallest cell extent that contains the given bounding box.
     */
    explicit CellExtent(const BoundingBox& boundingBox, float cellWidth,
                        float cellHeight);

    /**
     * Constructs the smallest cell extent that contains the given min max box.
     */
    explicit CellExtent(const MinMaxBox& box, float cellWidth,
                        float cellHeight);

    /**
     * Prints this extent's current values.
     */
    void print() const;
};

} // End namespace AM
