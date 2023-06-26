#pragma once

#include "Rotation.h"
#include "EmptySpriteID.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>

namespace AM
{
namespace SpriteEditor
{
/**
 * Holds the data necessary for editing and saving an object sprite set.
 * Part of SpriteDataModel. 
 */
struct EditorObjectSpriteSet {
    /** This sprite set's unique numeric identifier.
        Note: This ID may change when this sprite set is saved to the json. */
    Uint16 numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each sprite in this set.
        Objects support 8 directions of rotation. At least 1 sprite must be 
        set. If a direction isn't provided, it should be set to 
        EMPTY_SPRITE_ID. */
    std::array<int, Rotation::Direction::Count> spriteIDs{
        EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID,
        EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID, EMPTY_SPRITE_ID};
};

} // namespace SpriteEditor
} // namespace AM
