#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * A graphic set's numeric ID.
 *
 * These IDs aren't super useful since we cast to Uint16 all the time for 
 * generic code, but they at least give a solid indication that each graphic 
 * set type has its own ID space.
 *
 * If we ever care to, we can replace everywhere we cast to Uint16 with a 
 * variant that contains these types.
 */
using FloorGraphicSetID = Uint16;
using FloorCoveringGraphicSetID = Uint16;
using WallGraphicSetID = Uint16;
using ObjectGraphicSetID = Uint16;

/**
 * The ID of the "null graphic set", or the ID used to indicate that a graphic
 * set is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (graphicSetID)".
 */
static constexpr FloorGraphicSetID NULL_FLOOR_GRAPHIC_SET_ID{0};
static constexpr FloorCoveringGraphicSetID NULL_FLOOR_COVERING_GRAPHIC_SET_ID{0};
static constexpr WallGraphicSetID NULL_WALL_GRAPHIC_SET_ID{0};
static constexpr ObjectGraphicSetID NULL_OBJECT_GRAPHIC_SET_ID{0};

} // End namespace AM
