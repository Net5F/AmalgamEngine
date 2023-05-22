#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * The ID of the "empty sprite", or the ID used to indicate that a sprite is 
 * not present.
 *
 * One example of usage is in sprite sets: if an Object sprite set doesn't 
 * implement some rotations, those slots are to to the empty sprite.
 *
 * Placed in a separate file to avoid unnecessarily including heavy headers.
 */
static constexpr int EMPTY_SPRITE_ID{-1};

} // End namespace AM
