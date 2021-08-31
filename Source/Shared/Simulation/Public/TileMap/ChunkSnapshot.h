#pragma once

#include "TileSnapshot.h"
#include "SharedConfig.h"
#include <vector>
#include <array>
#include <string>

namespace AM
{

/**
 * Holds chunk data in a persistable form (palette IDs instead of pointers).
 *
 * Used in serialization/deserialization.
 */
struct ChunkSnapshot
{
public:
    /** Used as a "we should never hit this" cap on the number of IDs in a
        palette. Only checked in debug builds. */
    static constexpr unsigned int MAX_IDS = 1000;

    /** Holds the string IDs of all the sprites used in this chunk's tiles.
        Tile layers hold indices into this palette. */
    std::vector<std::string> palette;

    /** The tiles that make up this chunk. */
    std::array<TileSnapshot, SharedConfig::CHUNK_TILE_COUNT> tiles;

    /**
     * Returns the palette index for the given string.
     * If the string is not in the palette, it will be added.
     */
    unsigned int getPaletteIndex(const std::string& stringId)
    {
        for (unsigned int i = 0; i < palette.size(); ++i) {
            if (palette[i] == stringId) {
                return i;
            }
        }

        palette.push_back(stringId);
        return palette.size() - 1;
    }
};

template<typename S>
void serialize(S& serializer, ChunkSnapshot& testChunk)
{
    serializer.container(testChunk.palette, SharedConfig::CHUNK_TILE_COUNT, [](S& serializer, std::string& string) {
        serializer.text1b(string, ChunkSnapshot::MAX_IDS);
    });

    serializer.container(testChunk.tiles);
}

} // End namespace AM
