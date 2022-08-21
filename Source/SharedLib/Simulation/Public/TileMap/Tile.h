#pragma once

#include "Sprite.h"
#include "BoundingBox.h"
#include <vector>

namespace AM
{
/**
 * A 32x32-unit tile in the tile map.
 *
 * A tile consists of layers of sprites, which can be floors, grass, walls,
 * etc.
 *
 * Tiles contain no logic. If something on a tile requires logic, e.g. a tree
 * growing over time, it must have a system act upon it.
 */
struct Tile {
public:
    /**
     * Represents a sprite, placed on one of a tile's layers.
     */
    struct SpriteLayer {
        /** The sprite that this layer contains. */
        Sprite sprite{};

        /** If sprite.hasBoundingBox == true, this is the sprite's modelBounds
            moved to match the tile's world position.
            Tiles don't move, so we can calculate this ahead of time and save
            it here. */
        BoundingBox worldBounds{};
    };

    /** The layers of sprites that make up this tile, ordered bottom to top.

        Sprites with bounding boxes will be rendered in an order corresponding
        to their box extent, but sprites with no box will be rendered by order
        of appearance in this vector, from begin -> end. */
    std::vector<SpriteLayer> spriteLayers{};
};

} // End namespace AM
