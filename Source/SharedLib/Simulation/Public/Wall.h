#pragma once

#include <SDL_stdinc.h>

namespace AM
{
struct Wall {
    /**
     * The types of wall sprites that we use for our modular wall system.
     */
    enum Type : Uint8 {
        /** A West wall. */
        West,
        /** A North wall. */
        North,
        /** A column used to fill SouthEast-pointing corners. Gets placed on the
            tile southeast of the north and west walls that form the corner. */
        NorthWestGapFill,
        /** A 3/4 North wall used to fill NorthWest-pointing corners. Gets
           placed on the same tile as a west wall to form the corner. */
        NorthEastGapFill,
        /** The number of different wall types that we have. */
        Count,
        /** Used to tell if a tile layer doesn't contain a wall. */
        None
    };
};

} // End namespace AM
