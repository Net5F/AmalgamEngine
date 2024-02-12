#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * A sprite set's numeric ID.
 *
 * These IDs aren't super useful since we cast to Uint16 all the time for 
 * generic code, but they at least give a solid indication that each sprite 
 * set type has its own ID space.
 *
 * If we ever care to, we can replace everywhere we cast to Uint16 with a 
 * variant that contains these types.
 */
using FloorSpriteSetID = Uint16;
using FloorCoveringSpriteSetID = Uint16;
using WallSpriteSetID = Uint16;
using ObjectSpriteSetID = Uint16;

/**
 * The ID of the "null sprite set", or the ID used to indicate that a sprite 
 * set is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (spriteSetID)".
 */
static constexpr FloorSpriteSetID NULL_FLOOR_SPRITE_SET_ID{0};
static constexpr FloorCoveringSpriteSetID NULL_FLOOR_COVERING_SPRITE_SET_ID{0};
static constexpr WallSpriteSetID NULL_WALL_SPRITE_SET_ID{0};
static constexpr ObjectSpriteSetID NULL_OBJECT_SPRITE_SET_ID{0};

} // End namespace AM
