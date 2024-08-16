#pragma once

#include "DiscretePosition.h"
#include "HashTools.h"

namespace AM
{
struct Vector3;
struct TilePosition;

/**
 * A strong type alias, describing the position of a particular spatial
 * partitioning grid cell.
 */
struct CellPosition : public DiscretePosition<DiscreteImpl::CellTag> {
    CellPosition();

    CellPosition(int inX, int inY, int inZ);

    CellPosition(const DiscretePosition<DiscreteImpl::CellTag>& cellPosition);

    /**
     * Calculates the position of the cell that contains the given point.
     *
     * @param cellWidth The width of a cell, in world units.
     * @param cellHeight The height of a cell, in world units.
     */
    explicit CellPosition(const Vector3& worldPoint, float cellWidth,
                          float cellHeight);

    /**
     * Calculates the position of the cell that contains the given tile 
     * position.
     *
     * @param cellWidthTiles The width of a cell, in tiles.
     * @param cellHeightTiles The height of a cell, in tiles.
     */
    explicit CellPosition(const TilePosition& tilePosition,
                          std::size_t cellWidthTiles,
                          std::size_t cellHeightTiles);

    /**
     * Prints this position's current values.
     */
    void print() const;
};

} // namespace AM

// std::hash() specialization.
namespace std
{
template<>
struct hash<AM::CellPosition> {
    typedef AM::CellPosition argument_type;
    typedef std::size_t result_type;
    result_type operator()(const argument_type& position) const
    {
        std::size_t seed{0};
        AM::hash_combine(seed, position.x);
        AM::hash_combine(seed, position.y);
        AM::hash_combine(seed, position.z);
        return seed;
    }
};
} // namespace std
