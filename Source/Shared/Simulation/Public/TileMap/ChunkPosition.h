#pragma once

#include "DiscretePosition.h"

namespace AM
{

class TilePosition;

/**
 * A strong type alias, describing the position of a particular map chunk.
 */
class ChunkPosition : public DiscretePosition<DiscreteImpl::ChunkTag>
{
public:
    ChunkPosition();

    ChunkPosition(int inX, int inY);

    explicit ChunkPosition(const TilePosition& tilePosition);
};

template<typename S>
void serialize(S& serializer, ChunkPosition& chunkPosition)
{
    serializer.value4b(chunkPosition.x);
    serializer.value4b(chunkPosition.y);
}

} // namespace AM
