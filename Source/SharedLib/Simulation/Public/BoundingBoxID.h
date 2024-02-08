#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** A bounding box's numeric ID. */
using BoundingBoxID = Uint16;

/**
 * The ID used to indicate that a bounding box is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like 
 *       "if (boundingBoxID)".
 */
static constexpr BoundingBoxID NULL_BOUNDING_BOX_ID{0};

} // End namespace AM
