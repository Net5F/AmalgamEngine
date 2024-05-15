#pragma once

#include "DiscretePosition.h"

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
