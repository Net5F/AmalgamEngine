#pragma once

#include "Tile.h"
#include <array>
#include <fstream>

namespace AM
{
namespace Client
{
class SpriteData;

/**
 * Represents the world map.
 * Loads World.map and owns its data.
 *
 * Note: This class expects a World.map file to be present in the same
 *       directory as the application executable.
 *
 * The world map is composed of tiles, organized into 16x16 chunks.
 */
class TileMap
{
public:
    /**
     * Attempts to parse World.map and construct the tile map.
     *
     * Errors if World.map doesn't exist or it fails to parse.
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
    const Tile& get(int x, int y) const;

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
    unsigned long int getTileCount() const;

    /**
     * Saves the map to a file with the given name, placed in the same
     * directory as the program binary.
     */
    void save(const std::string& fileName);

private:
    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    static constexpr uint16_t MAP_FORMAT_VERSION = 0;

    /**
     * Parses the given data from a World.map file.
     *
     * Note: This will error if the data is invalid.
     */
    void parseMap(const std::vector<Uint8>& mapData);

    /**
     * Parses a single chunk, pushing its data into the tiles vector.
     *
     * @param mapData  The data from a loaded World.map file.
     * @param bufferIndex  An index into mapData, pointing at the start of the
     *                     desired chunk's data.
     * @param chunkX  The X index of this chunk.
     * @param chunkY  The Y index of this chunk.
     */
    void parseChunk(const std::vector<Uint8>& mapData, unsigned long& bufferIndex
                    , unsigned int chunkX, unsigned int chunkY);

    /** The length, in chunks, of the map's X axis. */
    unsigned int mapXLengthChunks;

    /** The length, in chunks, of the map's Y axis. */
    unsigned int mapYLengthChunks;

    /** The length, in tiles, of the map's X axis. */
    unsigned int mapXLengthTiles;

    /** The length, in tiles, of the map's Y axis. */
    unsigned int mapYLengthTiles;

    /** The total number of tiles that are in this map. */
    unsigned long int tileCount;

    /** The tiles that make up this map. */
    std::vector<Tile> tiles;

    /** Used to get sprites while constructing tiles. */
    SpriteData& spriteData;
};

} // End namespace Client
} // End namespace AM
