#pragma once

#include "TileLayer.h"
#include "SharedConfig.h"
#include "Log.h"
#include <vector>
#include <array>
#include <string>

namespace AM
{

/**
 * Holds chunk data in a persistable form (palette IDs instead of pointers).
 *
 * Used in saving/loading the tile map.
 */
struct ChunkSnapshot {
    struct PaletteEntry {
        /** The string ID of the graphic set that this entry refers to. */
        std::string graphicSetID{""};

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

    /** Used as a "we should never hit this" cap on the number of entries in a
        palette. */
    static constexpr std::size_t MAX_PALETTE_ENTRIES{1000};

    /** Used as a "we should never hit this" cap on the size of each ID string
        in the palette. */
    static constexpr std::size_t MAX_ID_LENGTH{50};

    /** Used as a "we should never hit this" cap on the number of tile layers 
        that a single chunk can contain. */
    static constexpr std::size_t MAX_TILE_LAYERS{256 * 10};

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
                                const std::string& graphicSetID,
                                Uint8 graphicValue)
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
void serialize(S& serializer, ChunkSnapshot::PaletteEntry& paletteEntry)
{
    serializer.text1b(paletteEntry.graphicSetID, ChunkSnapshot::MAX_ID_LENGTH);
    serializer.value1b(paletteEntry.layerType);
    serializer.value1b(paletteEntry.graphicValue);
}

template<typename S>
void serialize(S& serializer, ChunkSnapshot& chunkSnapshot)
{
    serializer.container(chunkSnapshot.palette,
                         ChunkSnapshot::MAX_PALETTE_ENTRIES);
    serializer.container1b(chunkSnapshot.tileLayerCounts);
    serializer.container1b(chunkSnapshot.tileLayers,
                           ChunkSnapshot::MAX_TILE_LAYERS);
    serializer.container(chunkSnapshot.tileOffsets,
                         ChunkSnapshot::MAX_TILE_LAYERS);
}

} // End namespace AM
