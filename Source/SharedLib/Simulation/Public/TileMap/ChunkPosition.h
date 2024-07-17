#pragma once

#include "DiscretePosition.h"
#include "HashTools.h"

namespace AM
{
struct Vector3;
struct TilePosition;

/**
 * A strong type alias, describing the position of a particular map chunk.
 */
struct ChunkPosition : public DiscretePosition<DiscreteImpl::ChunkTag> {
    ChunkPosition();

    ChunkPosition(int inX, int inY, int inZ);

    /**
     * Calculates the position of the chunk that contains the given point.
     */
    explicit ChunkPosition(const Vector3& worldPoint);

    explicit ChunkPosition(const TilePosition& tilePosition);

    /**
     * Prints this position's current values.
     */
    void print() const;
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
