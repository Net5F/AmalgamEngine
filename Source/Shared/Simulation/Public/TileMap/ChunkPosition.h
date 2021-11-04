#pragma once

#include "DiscretePosition.h"

namespace AM
{

/**
 * A strong type alias, describing the position of a particular map chunk.
 */
using ChunkPosition = DiscretePosition<ChunkTag>;

template<typename S>
void serialize(S& serializer, ChunkPosition& chunkPosition)
{
    serializer.value4b(chunkPosition.x);
    serializer.value4b(chunkPosition.y);
}

} // namespace AM
