#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of spells that an entity may cast.
 */
enum class SpellType : Uint8 {
    // Note: NotSet must always be present.
    NotSet = 0,
    Fireball
};

} // End namespace AM
