#pragma once

#include "DiscretePosition.h"
#include "HashTools.h"

namespace AM
{
struct Vector3;
struct ChunkPosition;

/**
 * A strong type alias, describing the position of a particular map tile.
 */
struct TilePosition : public DiscretePosition<DiscreteImpl::TileTag> {
    TilePosition();

    TilePosition(int inX, int inY, int inZ);

    /**
     * Calculates the position of the tile that contains the given point.
     */
    explicit TilePosition(const Vector3& worldPoint);

    explicit TilePosition(const ChunkPosition& chunkPosition);

    /** Returns the world point at this tile's origin (the least extreme 
        point along all axes). */
    Vector3 getOriginPoint() const;

    /** Returns the world point at the 3D center of this tile. */
    Vector3 getCenterPoint() const;

    /** Returns the world point centered in the X and Y axis, but at the 
        lowest Z position of this tile. */
    Vector3 getCenteredBottomPoint() const;

    /**
     * Prints this position's current values.
     */
    void print() const;
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
