#pragma once

#include <SDL2/SDL_stdinc.h>
#include <vector>

namespace AM
{

/**
 * Holds tile data in a persistable form (palette IDs instead of pointers).
 *
 * Used in saving/loading the tile map and in sending map data over the
 * network.
 */
struct TileSnapshot
{
public:
    /** Used as a "we should never hit this" cap on the number of layers in a
        tile. Only checked in debug builds. */
    static constexpr unsigned int MAX_SPRITE_LAYERS = 100;

    /** The sprites that make up this tile, stored bottom to top.
        Sprites are referred to by their index in the palette of the chunk that
        contains this tile. */
    std::vector<Uint8> spriteLayers;
};

template<typename S>
void serialize(S& serializer, TileSnapshot& testTile)
{
    serializer.container1b(testTile.spriteLayers, TileSnapshot::MAX_SPRITE_LAYERS);
}

} // End namespace AM
