#pragma once

#include "DiscretePosition.h"
#include "HashTools.h"

namespace AM
{
struct ChunkPosition;
struct Position;

/**
 * A strong type alias, describing the position of a particular map tile.
 */
struct TilePosition : public DiscretePosition<DiscreteImpl::TileTag>
{
    TilePosition();

    TilePosition(int inX, int inY);

    explicit TilePosition(const ChunkPosition& chunkPosition);

    /** Returns the world position at the center of this tile. */
    Position getCenterPosition();
};

} // namespace AM

// std::hash() specialization.
namespace std
{
template<>
struct hash<AM::TilePosition> {
    typedef AM::TilePosition argument_type;
    typedef std::size_t result_type;
    result_type operator()(const argument_type& position) const
    {
        std::size_t seed{0};
        AM::hash_combine(seed, position.x);
        AM::hash_combine(seed, position.y);
        return seed;
    }
};
} // namespace std
