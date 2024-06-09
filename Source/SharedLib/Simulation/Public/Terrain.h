#pragma once

#include <SDL_stdinc.h>

namespace AM
{
struct Terrain {
    /**
     * The types of terrain graphics that we use for our terrain system.
     */
    enum Type : Uint8 {
        /** A flat floor. */
        Flat,
        /** Cubic blocks. */
        BlockOneQuarter,
        BlockHalf,
        BlockThreeQuarter,
        BlockFull,
        /** South wedge. */
        WedgeSOneQuarter,
        WedgeSHalf,
        WedgeSThreeQuarter,
        WedgeSFull,
        /** SouthWest wedge. */
        WedgeSWOneQuarter,
        WedgeSWHalf,
        WedgeSWThreeQuarter,
        WedgeSWFull,
        /** West wedge. */
        WedgeWOneQuarter,
        WedgeWHalf,
        WedgeWThreeQuarter,
        WedgeWFull,
        /** NorthWest wedge. */
        WedgeNWOneQuarter,
        WedgeNWHalf,
        WedgeNWThreeQuarter,
        WedgeNWFull,
        /** North wedge. */
        WedgeNOneQuarter,
        WedgeNHalf,
        WedgeNThreeQuarter,
        WedgeNFull,
        /** NorthEast wedge. */
        WedgeNEOneQuarter,
        WedgeNEHalf,
        WedgeNEThreeQuarter,
        WedgeNEFull,
        /** East wedge. */
        WedgeEOneQuarter,
        WedgeEHalf,
        WedgeEThreeQuarter,
        WedgeEFull,
        /** SouthEast wedge. */
        WedgeSEOneQuarter,
        WedgeSEHalf,
        WedgeSEThreeQuarter,
        WedgeSEFull,
        /** The number of different terrain types that we have. */
        Count,
        /** Used to tell if a tile doesn't contain any terrain. */
        None
    };
};

} // End namespace AM
