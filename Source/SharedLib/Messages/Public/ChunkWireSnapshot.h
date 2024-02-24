#pragma once

#include "TileSnapshot.h"
#include "ChunkSnapshot.h"
#include "SharedConfig.h"
#include "Log.h"
#include <vector>
#include <array>

namespace AM
{
/**
 * Similar to ChunkSnapshot, but to save space uses the numeric ID of graphic 
 * in the palette instead of their string ID.
 *
 * Since the integer ID isn't persistable, this struct is only suitable for
 * sending chunk data to clients over the wire.
 */
struct ChunkWireSnapshot {
public:
    struct PaletteEntry {
        /** The type of tile layer that this entry represents. */
        TileLayer::Type layerType{TileLayer::Type::None};

        /** The numeric ID of the graphic set that this entry refers to. */
        Uint16 graphicSetID{0};

        /** The index within graphicSet.graphics that this entry refers to.
            For Walls, cast this to Wall::Type. For Floor Coverings and Objects,
            cast this to Rotation::Direction. For Floors, this will always be 0
            (floor graphic sets only have 1 graphic). */
        Uint8 graphicIndex{0};
    };

    /** This chunk's X-axis coordinate. */
    Uint16 x{0};

    /** This chunk's Y-axis coordinate. */
    Uint16 y{0};

    /** Holds an entry for each graphic used in this chunk's tiles. Part of a
        space-saving approach that lets TileSnapshot hold indices into this
        palette instead of directly holding the data. */
    std::vector<PaletteEntry> palette;

    /** The tiles that make up this chunk, stored in row-major order. */
    std::array<TileSnapshot, SharedConfig::CHUNK_TILE_COUNT> tiles;

    /**
     * Returns the palette index for the given palette entry info.
     * If the palette doesn't have a matching entry, it will be added.
     */
    std::size_t getPaletteIndex(TileLayer::Type tileLayerType,
                                Uint16 graphicSetID, Uint8 graphicIndex)
    {
        // TODO: If this gets to be a performance issue, we can look into
        //       switching palette to a map. Serialization will be more
        //       complicated, though.
        // Check if we already have this ID.
        for (std::size_t i = 0; i < palette.size(); ++i) {
            if ((palette[i].layerType == tileLayerType)
                && (palette[i].graphicSetID == graphicSetID)
                && (palette[i].graphicIndex == graphicIndex)) {
                // We already have the string, returns its index.
                return i;
            }
        }

        // We didn't have a matching entry, add it.
        if (palette.size() < UINT8_MAX) {
            palette.emplace_back(tileLayerType, graphicSetID, graphicIndex);
        }
        else {
            // TODO: If this becomes an issue, either switch to Uint16 or
            //       find some more efficient way to grow the space.
            LOG_FATAL("Ran out of palette slots.");
        }
        return (palette.size() - 1);
    }
};

template<typename S>
void serialize(S& serializer, ChunkWireSnapshot::PaletteEntry& paletteEntry)
{
    serializer.value1b(paletteEntry.layerType);
    serializer.value2b(paletteEntry.graphicSetID);
    serializer.value1b(paletteEntry.graphicIndex);
}

template<typename S>
void serialize(S& serializer, ChunkWireSnapshot& testChunk)
{
    serializer.value2b(testChunk.x);

    serializer.value2b(testChunk.y);

    serializer.container(testChunk.palette, ChunkSnapshot::MAX_PALETTE_ENTRIES);

    serializer.container(testChunk.tiles);
}

} // End namespace AM
