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
struct TilePosition : public DiscretePosition<DiscreteImpl::TileTag> {
    TilePosition();

    TilePosition(int inX, int inY, int inZ);

    explicit TilePosition(const ChunkPosition& chunkPosition);

    /** Returns the world position at this tile's origin (the least extreme 
        point along all axes). */
    Position getOriginPosition() const;

    /** Returns the world position at the 3D center of this tile. */
    Position getCenterPosition() const;

    /** Returns the world position centered in the X and Y axis, but at the 
        lowest Z position of this tile. */
    Position getCenteredBottomPosition() const;
};

template<typename S>
void serialize(S& serializer, TilePosition& tilePosition)
{
    serializer.value4b(tilePosition.x);
    serializer.value4b(tilePosition.y);
    serializer.value4b(tilePosition.z);
}

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
        AM::hash_combine(seed, position.z);
        return seed;
    }
};
} // namespace std
