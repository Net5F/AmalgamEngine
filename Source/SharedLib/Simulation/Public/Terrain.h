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

    /** The maximum value that height + startHeight can equal.
        Any higher value would push the terrain above the tile bounds. */
    static constexpr Uint8 MAX_COMBINED_HEIGHT{4};

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
     * Builds a Value out of the given terrain height and start height.
     */
    static Value toValue(Height height, Height startHeight);

    /**
     * Returns the given height value in Z-axis world units.
     */
    static float getHeightWorldValue(Height height);

    /**
     * Returns a bounding volume matching the given terrain value, translated to
     * world space and offset to the given tile coords.
     *
     * Note: Terrain always uses AABBs for collision volumes.
     */
    static BoundingBox calcWorldBounds(const TilePosition& tilePosition,
                                       Value value);
};

} // End namespace AM
