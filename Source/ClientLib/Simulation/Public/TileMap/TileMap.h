#pragma once

#include "TileMapBase.h"
#include "entt/signal/sigh.hpp"

namespace AM
{
namespace Client
{
class SpriteData;

/**
 * Owns and manages the world's tile map state.
 * Tiles are conceptually organized into 16x16 chunks.
 *
 * Tile map data is streamed from the server at runtime.
 */
class TileMap : public TileMapBase
{
public:
    /**
     * Attempts to parse TileMap.bin and construct the tile map.
     *
     * Errors if TileMap.bin doesn't exist or it fails to parse.
     */
    TileMap(SpriteData& inSpriteData);

    /**
     * Sets the size of the map and resizes the tiles vector.
     */
    void setMapSize(std::size_t inMapXLengthChunks,
                    std::size_t inMapYLengthChunks);

private:
    entt::sigh<void(TileExtent)> sizeChangedSig;

public:
    /** The tile map's size (extent) has changed. */
    entt::sink<entt::sigh<void(TileExtent)>> sizeChanged;
};

} // End namespace Client
} // End namespace AM
