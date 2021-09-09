#pragma once

#include "Tile.h"
#include "BinaryBuffer.h"
#include <array>
#include <fstream>

namespace AM
{
class TileMapSnapshot;
namespace Server
{
class SpriteData;

/**
 * Represents a tile map.
 * Loads TileMap.bin and owns its data.
 *
 * Note: This class expects a TileMap.bin file to be present in the same
 *       directory as the application executable.
 *
 * The map is composed of tiles, organized into 16x16 chunks.
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
     * Adds the given sprite to a new layer on the tile at the given position.
     *
     * Note: There is no bounds checking on the given values. It's on you to
     *       make sure they're valid.
     */
    void addSpriteLayer(unsigned int tileX, unsigned int tileY,
                        const Sprite& sprite);

    /**
     * Replaces the specified sprite layer with the given sprite.
     *
     * Note: There is no bounds checking on the given values. It's on you to
     *       make sure they're valid.
     */
    void replaceSpriteLayer(unsigned int tileX, unsigned int tileY,
                            unsigned int layerIndex, const Sprite& sprite);

    /**
     * Gets a const reference to the tile at the given coordinates.
     */
    const Tile& getTile(unsigned int x, unsigned int y) const;

    /**
     * Returns the length, in chunks, of the map's X axis.
     */
    unsigned int xLengthChunks() const;

    /**
     * Returns the length, in chunks, of the map's Y axis.
     */
    unsigned int yLengthChunks() const;

    /**
     * Returns the length, in tiles, of the map's X axis.
     */
    unsigned int xLengthTiles() const;

    /**
     * Returns the length, in tiles, of the map's Y axis.
     */
    unsigned int yLengthTiles() const;

    /**
     * Returns the total number of tiles in the map.
     */
    unsigned int getTileCount() const;

    /**
     * Saves the map to a file with the given name, placed in the same
     * directory as the program binary.
     */
    void save(const std::string& fileName);

private:
    /**
     * Returns the index in the tiles vector where the tile with the given
     * coordinates can be found.
     */
    inline unsigned int linearizeTileIndex(int x, int y) const
    {
        return (y * mapXLengthTiles) + x;
    }

    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    static constexpr uint16_t MAP_FORMAT_VERSION = 0;

    /**
     * Loads the given snapshot's data into this map.
     */
    void loadMap(TileMapSnapshot& mapSnapshot);

    /** The length, in chunks, of the map's X axis. */
    unsigned int mapXLengthChunks;

    /** The length, in chunks, of the map's Y axis. */
    unsigned int mapYLengthChunks;

    /** The length, in tiles, of the map's X axis. */
    unsigned int mapXLengthTiles;

    /** The length, in tiles, of the map's Y axis. */
    unsigned int mapYLengthTiles;

    /** The total number of tiles that are in this map. */
    unsigned int tileCount;

    /** The tiles that make up this map, stored in row-major order. */
    std::vector<Tile> tiles;

    /** Used to get sprites while constructing tiles. */
    SpriteData& spriteData;
};

} // End namespace Server
} // End namespace AM
