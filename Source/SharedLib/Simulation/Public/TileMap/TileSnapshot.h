#pragma once

#include <SDL_stdinc.h>
#include <vector>

namespace AM
{
/**
 * Holds tile data in a persistable form (palette IDs instead of pointers).
 *
 * Used in saving/loading the tile map and in sending map data over the
 * network.
 */
struct TileSnapshot {
    /** Used as a "we should never hit this" cap on the number of layers in a
        tile. */
    static constexpr std::size_t MAX_SPRITE_LAYERS{100};

    /** The layers of sprites that make up this tile, stored bottom to top.

        This vector's elements are indices into the palette of the ChunkSnapshot
        or ChunkWireSnapshot that contains this tile snapshot. */
    std::vector<Uint8> layers{};
};

template<typename S>
void serialize(S& serializer, TileSnapshot& tileSnapshot)
{
    serializer.container1b(tileSnapshot.layers,
                           TileSnapshot::MAX_SPRITE_LAYERS);
}

} // End namespace AM
