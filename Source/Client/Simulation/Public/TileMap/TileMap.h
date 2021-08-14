#pragma once

#include "Tile.h"
#include <array>

namespace AM
{
namespace Client
{
class SpriteData;

/**
 * Represents the world map.
 * Loads World.map and owns its data.
 *
 * The world map is composed of tiles, organized into 16x16 chunks.
 */
class TileMap {
public:
    /**
     * Parses World.map to construct the map.
     */
    TileMap(SpriteData& inSpriteData);

    /**
     * Adds the given sprite to a new layer on the tile at the given position.
     *
     * Note: There is no bounds checking on the given values. It's on you to
     *       make sure they're valid.
     */
    void addSpriteLayer(unsigned int tileX, unsigned int tileY, const Sprite& sprite);

    /**
     * Replaces the specified sprite layer with the given sprite.
     *
     * Note: There is no bounds checking on the given values. It's on you to
     *       make sure they're valid.
     */
    void replaceSpriteLayer(unsigned int tileX, unsigned int tileY
                            , unsigned int layerIndex, const Sprite& sprite);

    /**
     * Gets a const reference to the tile at the given coordinates.
     */
    const Tile& get(int x, int y) const;

    /**
     * Returns the total number of tiles in the map.
     */
    std::size_t size();

    /**
     * Returns the number of tiles per row in the map.
     */
    std::size_t sizeX();

    /**
     * Returns the numbers of tiles per column in the map.
     */
    std::size_t sizeY();

private:
    /** The number of tiles that are in this world map. */
    static constexpr std::size_t TILE_COUNT = SharedConfig::WORLD_WIDTH * SharedConfig::WORLD_HEIGHT;

    /** The tiles that make up the world map. */
    std::array<Tile, TILE_COUNT> tiles;

    /** Used to get sprites while constructing tiles. */
    SpriteData& spriteData;
};

} // End namespace Client
} // End namespace AM
