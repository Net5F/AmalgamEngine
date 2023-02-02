#pragma once

#include "MessageType.h"
#include <vector>

namespace AM
{

// We use a Uint8 for the layerCount in the below message, so we need to make
// sure that the max layer count is less than the Uint8 max.
static_assert(SharedConfig::MAX_TILE_LAYERS < SDL_MAX_UINT8);

/**
 * Used by the server to stream updated tile data to a client.
 */
struct TileUpdate {
public:
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE{MessageType::TileUpdate};

    /** Used as a "we should never hit this" cap on the number of tile updates
        we send at once. Only checked in debug builds. */
    static constexpr std::size_t MAX_UPDATED_TILES{
        SharedConfig::CHUNK_TILE_COUNT * 9};

    struct TileInfo {
        /** The X coordinate of the tile to update. */
        int tileX{0};

        /** The Y coordinate of the tile to update. */
        int tileY{0};

        /** The number of layer IDs in updatedLayers for this tile.
            Note: If this update only contains erased layers, this will be 0. */
        Uint8 layerCount{0};

        /** The layer index of the first ID in updatedLayers for this tile.
            Subsequent tiles will be incrementally ascending.
            When a layer is updated, we send that layer and all layers above
            it (so the client knows if upper layers were cleared).
            When an update only contains erased layers, this will be the lowest
            erased layer index. */
        Uint8 startLayerIndex{0};
    };

    /** Holds tile info, in the same order as updatedLayers. */
    std::vector<TileInfo> tileInfo;

    /** The numericID that each layer is now set to.
        Ordered to match tileInfo. E.g. if tileInfo[0] == {2, 2, 3, 1}, the
        first 3 elements in this vector correspond to tile (2, 2) and are
        ordered by increasing layer index starting at 1: (1, 2, 3).

        To find which tile and layer an ID in this vector corresponds to,
        iterate tileInfo and increment based on each layerCount.

        Note: Erased layers are not added to this vector. */
    std::vector<int> updatedLayers;
};

template<typename S>
void serialize(S& serializer, TileUpdate::TileInfo& tileInfo)
{
    serializer.value4b(tileInfo.tileX);
    serializer.value4b(tileInfo.tileY);
    serializer.value1b(tileInfo.layerCount);
    serializer.value1b(tileInfo.startLayerIndex);
}

template<typename S>
void serialize(S& serializer, TileUpdate& tileUpdate)
{
    serializer.container(tileUpdate.tileInfo, TileUpdate::MAX_UPDATED_TILES);
    serializer.container4b(
        tileUpdate.updatedLayers,
        (TileUpdate::MAX_UPDATED_TILES
         * static_cast<std::size_t>(SharedConfig::MAX_TILE_LAYERS)));
}

} // End namespace AM
