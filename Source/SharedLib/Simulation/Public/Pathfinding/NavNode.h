#pragma once

#include "AMAssert.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * A single node in the navigation grid. See NavGrid.h.
 *
 * Each node describes the walkable surface within one tile (if it has one),
 * and how agents can stand on it and move across its edges.
 *
 * Heights are measured in "thirds" of a tile (TILE_WORLD_HEIGHT / 3),
 * matching our terrain heights.
 *
 * Bit layout:
 *   0-1   Surface height
 *           The height of the walkable surface within this tile, in thirds
 *           (0 - 2). A surface at the very top of a tile is instead stored in
 *           the tile above it, at height 0.
 *   2-4   Clearance
 *           The width, in tiles, of the largest square of standable nodes that
 *           is anchored at this node and extends east and south. All nodes in
 *           the square have the same surface height, and there are no blocked
 *           edges between them.
 *           0 == this node isn't standable.
 *   5-7   Headroom
 *           The free space above the surface, in thirds (rounded down).
 *           7 == 7 or more.
 *   8     West blocked
 *           The edge between this tile and its west neighbor is blocked.
 *   9     North blocked
 *           The edge between this tile and its north neighbor is blocked.
 *   10-15 Spare
 *
 * Note: To check the east or south edge of a tile, check the west or north
 *       edge of its neighbor.
 */
struct NavNode {
    static constexpr Uint8 MAX_SURFACE_HEIGHT{2};
    static constexpr Uint8 MAX_CLEARANCE{7};
    static constexpr Uint8 MAX_HEADROOM{7};

    /** The bit-packed node data. */
    Uint16 value{0};

    Uint8 getSurfaceHeight() const
    {
        return static_cast<Uint8>(value & SURFACE_HEIGHT_MASK);
    }

    Uint8 getClearance() const
    {
        return static_cast<Uint8>((value & CLEARANCE_MASK) >> CLEARANCE_SHIFT);
    }

    Uint8 getHeadroom() const
    {
        return static_cast<Uint8>((value & HEADROOM_MASK) >> HEADROOM_SHIFT);
    }

    bool isWestBlocked() const { return (value & WEST_BLOCKED_MASK) != 0; }

    bool isNorthBlocked() const { return (value & NORTH_BLOCKED_MASK) != 0; }

    /** Returns true if an agent can stand on this node. */
    bool isStandable() const { return (value & CLEARANCE_MASK) != 0; }

    void setSurfaceHeight(Uint8 surfaceHeight)
    {
        AM_ASSERT(surfaceHeight <= MAX_SURFACE_HEIGHT,
                  "Invalid surface height.");
        value = static_cast<Uint16>((value & ~SURFACE_HEIGHT_MASK)
                                    | surfaceHeight);
    }

    void setClearance(Uint8 clearance)
    {
        AM_ASSERT(clearance <= MAX_CLEARANCE, "Invalid clearance.");
        value = static_cast<Uint16>((value & ~CLEARANCE_MASK)
                                    | (clearance << CLEARANCE_SHIFT));
    }

    void setHeadroom(Uint8 headroom)
    {
        AM_ASSERT(headroom <= MAX_HEADROOM, "Invalid headroom.");
        value = static_cast<Uint16>((value & ~HEADROOM_MASK)
                                    | (headroom << HEADROOM_SHIFT));
    }

    void setWestBlocked(bool westBlocked)
    {
        value = static_cast<Uint16>(westBlocked ? (value | WEST_BLOCKED_MASK)
                                                : (value & ~WEST_BLOCKED_MASK));
    }

    void setNorthBlocked(bool northBlocked)
    {
        value
            = static_cast<Uint16>(northBlocked ? (value | NORTH_BLOCKED_MASK)
                                               : (value & ~NORTH_BLOCKED_MASK));
    }

    bool operator==(const NavNode& other) const = default;

private:
    static constexpr Uint16 SURFACE_HEIGHT_MASK{0b0000'0000'0000'0011};
    static constexpr Uint16 CLEARANCE_MASK{0b0000'0000'0001'1100};
    static constexpr Uint16 CLEARANCE_SHIFT{2};
    static constexpr Uint16 HEADROOM_MASK{0b0000'0000'1110'0000};
    static constexpr Uint16 HEADROOM_SHIFT{5};
    static constexpr Uint16 WEST_BLOCKED_MASK{0b0000'0001'0000'0000};
    static constexpr Uint16 NORTH_BLOCKED_MASK{0b0000'0010'0000'0000};
};

} // End namespace AM
