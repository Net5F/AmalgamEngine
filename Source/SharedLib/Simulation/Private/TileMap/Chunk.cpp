#include "Chunk.h"
#include "Morton.h"
#include "AMAssert.h"
#include <cstdint>

namespace AM
{

Tile& Chunk::getTile(Uint16 tileOffsetX, Uint16 tileOffsetY)
{
    AM_ASSERT(tileOffsetX < tiles.size(), "Invalid tile offset.");
    AM_ASSERT(tileOffsetY < tiles.size(), "Invalid tile offset.");
    return tiles[mortonEncode(tileOffsetX, tileOffsetY)];
}

const Tile& Chunk::getTile(Uint16 tileOffsetX, Uint16 tileOffsetY) const
{
    AM_ASSERT(tileOffsetX < tiles.size(), "Invalid tile offset.");
    AM_ASSERT(tileOffsetY < tiles.size(), "Invalid tile offset.");
    return tiles[mortonEncode(tileOffsetX, tileOffsetY)];
}

Uint32 Chunk::mortonEncode(Uint16 x, Uint16 y) const
{
    // If x and y fit in our lookup table, use it. Otherwise, calculate it 
    // at runtime.
    if constexpr (SharedConfig::CHUNK_WIDTH <= 16) {
        return Morton::encode16x16(static_cast<Uint8>(x),
                                        static_cast<Uint8>(y));
    }
    else {
        return Morton::encode32(x, y);
    }
}

} // End namespace AM
