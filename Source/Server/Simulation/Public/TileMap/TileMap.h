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
namespace Server
{
class SpriteData;

/**
 * Owns and manages the world's tile map state.
 * Tiles are conceptually organized into 16x16 chunks.
 *
 * Persisted tile map data is loaded from TileMap.bin.
 *
 * Note: This class expects a TileMap.bin file to be present in the same
 *       directory as the application executable.
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
     * Attempts to save the current tile map state to TileMap.bin.
     */
    ~TileMap();

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
    void setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
                            unsigned int layerIndex, const Sprite& sprite);

    /**
     * Overload for sprite string IDs.
     */
    void setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
                            unsigned int layerIndex,
                            const std::string& stringID);

    /**
     * Overload for sprite numeric IDs.
     */
    void setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
                            unsigned int layerIndex, int numericID);

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

    /**
     * Saves the map to a file with the given name, placed in the same
     * directory as the program binary.
     *
     * @param fileName  The file name to save to, with no path prepended.
     */
    void save(const std::string& fileName);

private:
    /**
     * Returns the index in the tiles vector where the tile with the given
     * coordinates can be found.
     */
    inline unsigned int linearizeTileIndex(int x, int y) const
    {
        return (y * tileExtent.xLength) + x;
    }

    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    static constexpr uint16_t MAP_FORMAT_VERSION = 0;

    /**
     * Loads the given snapshot's data into this map.
     */
    void loadMap(TileMapSnapshot& mapSnapshot);

    /** The map's extent, with chunks as the unit. */
    ChunkExtent chunkExtent;

    /** The map's extent, with tiles as the unit. */
    TileExtent tileExtent;

    /** The tiles that make up this map, stored in row-major order. */
    std::vector<Tile> tiles;

    /** Used to get sprites while constructing tiles. */
    SpriteData& spriteData;
};

} // End namespace Server
} // End namespace AM
