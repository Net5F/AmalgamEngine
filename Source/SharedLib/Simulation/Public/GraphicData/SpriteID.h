#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** A sprite's numeric ID. */
using SpriteID = Uint32;

/**
 * The ID of the "null sprite", or the ID used to indicate that a sprite is
 * not present.
 *
 * One example of usage is in sprite sets: if an Object sprite set doesn't
 * implement some rotations, those slots are set to the null sprite.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (spriteID)".
 */
static constexpr SpriteID NULL_SPRITE_ID{0};

} // End namespace AM
