#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** A sprite sheet's numeric ID. */
using SpriteSheetID = Uint32;

/**
 * The ID of the "null sprite sheet", or the ID used to indicate that a sprite 
 * sheet is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like 
 *       "if (spriteSheetID)".
 */
static constexpr SpriteSheetID NULL_SPRITE_SHEET_ID{0};

} // End namespace AM
