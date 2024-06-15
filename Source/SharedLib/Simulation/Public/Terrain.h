#pragma once

#include "BoundingBox.h"
#include <SDL_stdinc.h>

namespace AM
{
struct TilePosition;

struct Terrain {
    /**
     * A terrain value is made up of 2 parts:
     *   Block Height (4b)
     *     How tall the terrain block is, relative to our tile height.
     *   Start Height (4b)
     *     How high the terrain block should be placed within the tile.
     *
     * Note: Terrain must never extend beyond the bounds of its tile.
     */
    using Value = Uint8;

    /**
     * A Z-axis height value, relative to SharedConfig::TILE_WORLD_HEIGHT.
     *
     * Since we only have one terrain shape, this is enough information to 
     * describe which graphic to use for a given piece of terrain.
     */
    enum Height : Uint8 {
        Flat,
        OneThird,
        TwoThird,
        Full,
        Count
    };

    /** Bitmask for getting the height of the terrain. */
    static constexpr Uint8 HEIGHT_MASK{0b11110000};
    /** Bitmask for getting the starting height of the terrain. */
    static constexpr Uint8 START_HEIGHT_MASK{0b00001111};

    /**
     * Returns the height field from the given terrain value.
     */
    static Height getHeight(Value value);

    /**
     * Returns the start height field from the given terrain value.
     */
    static Height getStartHeight(Value value);

    struct InfoReturn
    {
        /** The terrain's height. */
        Height height{};
        /** The terrain's starting height. */
        Height startHeight{};
    };
    /**
     * Returns the separate terrain info fields that are bitpacked in the 
     * given value.
     */
    static InfoReturn getInfo(Value value);

    /**
     * Returns an appropriate bounding box for the given terrain info.
     */
    static BoundingBox getWorldBounds(const TilePosition& tilePosition,
                                      Height height, Height startHeight);
};

} // End namespace AM
