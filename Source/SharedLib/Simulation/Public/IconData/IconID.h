#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** An icon's numeric ID. */
using IconID = Uint16;

/**
 * The ID of the "null icon", or the ID used to indicate that an icon is 
 * not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (iconID)".
 */
static constexpr IconID NULL_ICON_ID{0};

} // End namespace AM
