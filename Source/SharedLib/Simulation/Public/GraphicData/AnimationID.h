#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** An animation's numeric ID. */
using AnimationID = Uint16;

/**
 * The ID of the "null animation", or the ID used to indicate that an animation 
 * is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (animationID)".
 */
static constexpr AnimationID NULL_ANIMATION_ID{0};

} // End namespace AM
