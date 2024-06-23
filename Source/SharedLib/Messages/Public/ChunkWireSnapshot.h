#pragma once

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
        /** The numeric ID of the graphic set that this entry refers to. */
        Uint16 graphicSetID{0};

        /** The type of tile layer that this entry represents.
            Each layer type maps directly to a single graphic set type. */
        TileLayer::Type layerType{TileLayer::Type::None};

        /** The layer's graphic value.
            For all types except Terrain, this is simply an index into 
            graphicSet.graphics. For Terrain, this is a bit-packed value.
            For Terrain, cast this to Terrain::Value. For Walls, cast this to 
            Wall::Type. For Floors and Objects, cast this to 
            Rotation::Direction. */
        Uint8 graphicValue{0};
    };

    /** This chunk's X-axis coordinate. */
    Sint16 x{0};

    /** This chunk's Y-axis coordinate. */
    Sint16 y{0};

    /** This chunk's Z-axis coordinate. */
    Sint16 z{0};

    /** Holds an entry for each graphic used in this chunk's tiles. Part of a
        space-saving approach that lets TileSnapshot hold indices into this
        palette instead of directly holding the data. */
    std::vector<PaletteEntry> palette{};

    /** The number of layers that each tile in this chunk has, stored in row-
        major order. */
    std::array<Uint8, SharedConfig::CHUNK_TILE_COUNT> tileLayerCounts{};

    /** This vector's elements are indices into the palette, each index  
        representing a tile layer that is owned by a tile in this chunk.
        These layers are ordered by tile coordinate in morton order, and by 
        the usual bottom-to-top type order within each tile.
        To iterate, use tileLayerCounts to determine how many layers belong to 
        each tile. */
    std::vector<Uint8> tileLayers{};

    /** The tile offset for each Floor and Object tile layer in tileLayers, 
        stored in the order that they'll be encountered while iterating. */
    std::vector<TileOffset> tileOffsets{};

    /**
     * Returns the palette index for the given palette entry info.
     * If the palette doesn't have a matching entry, it will be added.
     */
    std::size_t getPaletteIndex(TileLayer::Type tileLayerType,
                                Uint16 graphicSetID, Uint8 graphicValue)
    {
        // TODO: If this gets to be a performance issue, we can look into
        //       switching palette to a map. Serialization will be more
        //       complicated, though.
        // Check if we already have this ID.
        for (std::size_t i = 0; i < palette.size(); ++i) {
            if ((palette[i].layerType == tileLayerType)
                && (palette[i].graphicSetID == graphicSetID)
                && (palette[i].graphicValue == graphicValue)) {
                // We already have the string, returns its index.
                return i;
            }
        }

        // We didn't have a matching entry, add it.
        if (palette.size() < UINT8_MAX) {
            palette.emplace_back(graphicSetID, tileLayerType, graphicValue);
        }
        else {
            // TODO: If this becomes an issue, either switch to Uint16 or
            //       find some more efficient way to grow the space.
            LOG_ERROR("Ran out of palette slots.");
            return 0;
        }
        return (palette.size() - 1);
    }
};

template<typename S>
void serialize(S& serializer, ChunkWireSnapshot::PaletteEntry& paletteEntry)
{
    serializer.value2b(paletteEntry.graphicSetID);
    serializer.value1b(paletteEntry.layerType);
    serializer.value1b(paletteEntry.graphicValue);
}

template<typename S>
void serialize(S& serializer, ChunkWireSnapshot& chunkSnapshot)
{
    serializer.value2b(chunkSnapshot.x);
    serializer.value2b(chunkSnapshot.y);
    serializer.value2b(chunkSnapshot.z);
    serializer.container(chunkSnapshot.palette,
                         ChunkSnapshot::MAX_PALETTE_ENTRIES);
    serializer.container1b(chunkSnapshot.tileLayerCounts);
    serializer.container1b(chunkSnapshot.tileLayers,
                           ChunkSnapshot::MAX_TILE_LAYERS);
    serializer.container(chunkSnapshot.tileOffsets,
                         ChunkSnapshot::MAX_TILE_LAYERS);
}

} // End namespace AM
