#pragma once

#include "TileMapBase.h"
#include "entt/signal/sigh.hpp"
#include <SDL_stdinc.h>

namespace AM
{
class CollisionLocator;

namespace Client
{
class GraphicData;

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
    TileMap(const GraphicData& inGraphicData,
            CollisionLocator& inCollisionLocator);

    /**
     * Sets the size of the map and resizes the chunks vector.
     */
    void setMapSize(Uint16 inMapXLengthChunks, Uint16 inMapYLengthChunks,
                    Uint16 inMapZLengthChunks);

private:
    entt::sigh<void(TileExtent)> sizeChangedSig;

public:
    /** The tile map's size (extent) has changed. */
    entt::sink<entt::sigh<void(TileExtent)>> sizeChanged;
};

} // End namespace Client
} // End namespace AM
