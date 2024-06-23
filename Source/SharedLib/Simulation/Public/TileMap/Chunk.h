#pragma once

#include "Tile.h"
#include "SharedConfig.h"
#include <SDL_stdinc.h>
#include <array>

namespace AM
{
struct Sprite;

/**
 * A 32x32-unit tile in the tile map.
 *
 * A tile consists of layers of sprites, which can be floors, walls, etc.
 *
 * Tiles contain no logic. If something on a tile requires logic, e.g. a tree
 * growing over time, it must have a system act upon it.
 */
class Chunk
{
public:
    /** The number of tiles in the tiles array that are non-empty.
        Used to tell when this chunk is empty and can be deleted. */
    Uint16 tileLayerCount{0};

    /** The tiles that make up this chunk, stored in morton order. */
    std::array<Tile, SharedConfig::CHUNK_TILE_COUNT> tiles{};

    /**
     * Returns the tile at the given tile coordinate offset (with respect to 
     * this chunk's origin).
     */
    Tile& getTile(Uint16 tileOffsetX, Uint16 tileOffsetY);
    const Tile& getTile(Uint16 tileOffsetX, Uint16 tileOffsetY) const;

private:
    /**
     * Returns a morton code for the given x and y.
     * We use morton codes to lay out our tiles in a more cache-friendly way 
     * since we're likely to be accessing neighbors at the same time.
     */
    Uint32 mortonEncode32(Uint16 x, Uint16 y) const;
};

} // End namespace AM
