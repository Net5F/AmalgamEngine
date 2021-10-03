#pragma once

#include "TileSnapshot.h"
#include "ChunkSnapshot.h"
#include "SharedConfig.h"
#include <vector>
#include <array>

namespace AM
{
/**
 * Similar to ChunkSnapshot, but to save space uses the numeric ID of sprites
 * in the palette instead of their string ID.
 *
 * Since the integer ID isn't persistable, this struct is only suitable for
 * sending chunk data to clients.
 */
struct ChunkWireSnapshot {
public:
    /** The chunk's X-axis coordinate. */
    Uint16 x{0};

    /** The chunk's Y-axis coordinate. */
    Uint16 y{0};

    /** Holds the numeric IDs of all the sprites used in this chunk's tiles.
        Tile layers hold indices into this palette. */
    std::vector<int> palette;

    /** The tiles that make up this chunk, stored in row-major order. */
    std::array<TileSnapshot, SharedConfig::CHUNK_TILE_COUNT> tiles;

    /**
     * Returns the palette index for the given ID.
     * If the ID is not in the palette, it will be added.
     */
    unsigned int getPaletteIndex(int numericID)
    {
        // TODO: If this gets to be a performance issue, we can look into
        //       switching palette to a map. Serialization will be more
        //       complicated, though.

        // Check if we already have this id.
        for (unsigned int i = 0; i < palette.size(); ++i) {
            if (palette[i] == numericID) {
                // We already have the string, return its index.
                return i;
            }
        }

        // We didn't have the id, add it.
        palette.push_back(numericID);
        return (palette.size() - 1);
    }
};

template<typename S>
void serialize(S& serializer, ChunkWireSnapshot& testChunk)
{
    serializer.value2b(testChunk.x);

    serializer.value2b(testChunk.y);

    serializer.container4b(testChunk.palette, ChunkSnapshot::MAX_IDS);

    serializer.container(testChunk.tiles);
}

} // End namespace AM
