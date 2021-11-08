#pragma once

#include "Tile.h"
#include "BinaryBuffer.h"
#include "ChunkExtent.h"
#include "TileExtent.h"
#include <array>
#include <fstream>

namespace AM
{
class TileMapSnapshot;
namespace Client
{
class SpriteData;

/**
 * Owns and manages the world's tile map state.
 * Tiles are conceptually organized into 16x16 chunks.
 *
 * Tile map data is streamed from the server at runtime.
 *
 * TODO: Server::TileMap and Client::TileMap use different Sprite classes (
 *       and those Sprite classes are fundamentally different), so they can't
 *       be a single shared class. However, they do share a lot of code, so
 *       we should find a way to factor that code out into a shared class.
 */
class TileMap
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
    void setMapSize(unsigned int inMapXLengthChunks,
                    unsigned int inMapYLengthChunks);

    /**
     * Sets the specified sprite layer to the given sprite.
     *
     * If the specified tile's spriteLayers vector isn't big enough, resizes
     * it. Any tiles added during resizing will be default initialized to
     * the "empty sprite".
     *
     * Note: There's no bounds checking on tileX/tileY. It's on you to make
     *       sure they're valid.
     */
    void setSpriteLayer(unsigned int tileX, unsigned int tileY,
                        unsigned int layerIndex, const Sprite& sprite);

    /**
     * Overload for sprite string IDs.
     */
    void setSpriteLayer(unsigned int tileX, unsigned int tileY,
                        unsigned int layerIndex, const std::string& stringID);

    /**
     * Overload for sprite numeric IDs.
     */
    void setSpriteLayer(unsigned int tileX, unsigned int tileY,
                        unsigned int layerIndex, int numericID);

    /**
     * Clears all sprite layers out of the tile at the given index.
     */
    void clearTile(unsigned int tileX, unsigned int tileY);

    /**
     * Gets a const reference to the tile at the given coordinates.
     */
    const Tile& getTile(unsigned int x, unsigned int y) const;

    /**
     * Returns the map extent, with chunks as the unit.
     */
    const ChunkExtent& getChunkExtent() const;

    /**
     * Returns the map extent, with tiles as the unit.
     */
    const TileExtent& getTileExtent() const;

private:
    /**
     * Returns the index in the tiles vector where the tile with the given
     * coordinates can be found.
     */
    inline unsigned int linearizeTileIndex(int x, int y) const
    {
        return (y * tileExtent.xLength) + x;
    }

    /** The map's extent, with chunks as the unit. */
    ChunkExtent chunkExtent;

    /** The map's extent, with tiles as the unit. */
    TileExtent tileExtent;

    /** The tiles that make up this map, stored in row-major order. */
    std::vector<Tile> tiles;

    /** Used to get sprites while constructing tiles. */
    SpriteData& spriteData;
};

} // End namespace Client
} // End namespace AM
