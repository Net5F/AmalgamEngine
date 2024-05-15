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
    return tiles[mortonEncode32(tileOffsetX, tileOffsetY)];
}

const Tile& Chunk::getTile(Uint16 tileOffsetX, Uint16 tileOffsetY) const
{
    AM_ASSERT(tileOffsetX < tiles.size(), "Invalid tile offset.");
    AM_ASSERT(tileOffsetY < tiles.size(), "Invalid tile offset.");
    return tiles[mortonEncode32(tileOffsetX, tileOffsetY)];
}

Uint32 Chunk::mortonEncode32(Uint16 x, Uint16 y) const
{
    // If x and y fit in our lookup table, use it. Otherwise, calculate it 
    // at runtime.
    if constexpr (SharedConfig::CHUNK_WIDTH <= 16) {
        return Morton::m2D_lookup_16x16(static_cast<Uint8>(x),
                                        static_cast<Uint8>(y));
    }
    else {
        return Morton::m2D_e_magicbits_combined(x, y);
    }
}

} // End namespace AM
