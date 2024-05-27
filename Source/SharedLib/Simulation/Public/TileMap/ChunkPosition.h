#pragma once

#include "DiscretePosition.h"
#include "HashTools.h"

namespace AM
{
struct TilePosition;

/**
 * A strong type alias, describing the position of a particular map chunk.
 */
struct ChunkPosition : public DiscretePosition<DiscreteImpl::ChunkTag> {
    ChunkPosition();

    ChunkPosition(int inX, int inY, int inZ);

    explicit ChunkPosition(const TilePosition& tilePosition);
};

template<typename S>
void serialize(S& serializer, ChunkPosition& chunkPosition)
{
    serializer.value4b(chunkPosition.x);
    serializer.value4b(chunkPosition.y);
    serializer.value4b(chunkPosition.z);
}

} // namespace AM

// std::hash() specialization.
namespace std
{
template<>
struct hash<AM::ChunkPosition> {
    typedef AM::ChunkPosition argument_type;
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
