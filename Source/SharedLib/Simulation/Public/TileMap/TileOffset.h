#pragma once

#include "Terrain.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * An integer offset, relative to a tile coordinate.
 *
 * Among other uses, Floor and Object tile layers use this offset to support 
 * them being placed anywhere within a tile. Terrain and Walls can't be offset 
 * (Terrain is always aligned to the tile, Walls are always aligned to the top
 * face of the terrain).
 *
 * This offset will never be greater than the size of a tile.
 *
 * Note: Since world units are normally floats, this offset has limited 
 *       resolution. This is fine in our use cases.
 */
struct TileOffset {
    /** The X-axis world unit offset. Max == SharedConfig::TILE_WORLD_WIDTH */
    Uint8 x{0};

    /** The Y-axis world unit offset. Max == SharedConfig::TILE_WORLD_WIDTH */
    Uint8 y{0};

    /** The Z-axis world unit offset. Max == SharedConfig::TILE_WORLD_HEIGHT */
    Uint8 z{0};

    bool operator==(const TileOffset& other) const
    {
        return (x == other.x) && (y == other.y) && (z == other.z);
    }

    bool operator!=(const TileOffset& other) const
    {
        return (x != other.x) || (y != other.y) || (z != other.z);
    }
};

template<typename S>
void serialize(S& serializer, TileOffset& tileOffset)
{
    serializer.value1b(tileOffset.x);
    serializer.value1b(tileOffset.y);
    serializer.value1b(tileOffset.z);
}

} // End namespace AM
