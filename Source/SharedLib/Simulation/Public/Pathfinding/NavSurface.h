#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * A walkable surface within a column of tiles. See NavGrid.h.
 *
 * Heights are measured in "thirds" of a tile (TILE_WORLD_HEIGHT / 3),
 * matching our terrain heights.
 */
struct NavSurface {
    static constexpr Uint8 MAX_HEADROOM{SDL_MAX_UINT8};

    /** The surface's height, in thirds, above the bottom of the nav grid. */
    Uint16 height{0};

    /** The free space above the surface, in thirds (rounded down, and capped
        at MAX_HEADROOM). Always at least 1. */
    Uint8 headroom{0};

    /** Bit-packed flags. See the getters below. */
    Uint8 flags{0};

    /** If true, the edge between this surface's tile and its west neighbor
        is blocked (e.g. by a wall).
        Note: To check the east or south edge of a tile, check the west or
              north edge of its neighbor. */
    bool isWestBlocked() const { return (flags & WEST_BLOCKED_MASK) != 0; }

    /** If true, the edge between this surface's tile and its north neighbor
        is blocked. */
    bool isNorthBlocked() const { return (flags & NORTH_BLOCKED_MASK) != 0; }

    void setWestBlocked(bool westBlocked)
    {
        flags = static_cast<Uint8>(westBlocked ? (flags | WEST_BLOCKED_MASK)
                                               : (flags & ~WEST_BLOCKED_MASK));
    }

    void setNorthBlocked(bool northBlocked)
    {
        flags
            = static_cast<Uint8>(northBlocked ? (flags | NORTH_BLOCKED_MASK)
                                              : (flags & ~NORTH_BLOCKED_MASK));
    }

    bool operator==(const NavSurface& other) const = default;

private:
    static constexpr Uint8 WEST_BLOCKED_MASK{0b0000'0001};
    static constexpr Uint8 NORTH_BLOCKED_MASK{0b0000'0010};
};

} // End namespace AM
