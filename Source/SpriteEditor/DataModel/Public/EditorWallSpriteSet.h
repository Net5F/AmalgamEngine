#pragma once

#include "Wall.h"
#include "NullSpriteID.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>

namespace AM
{
namespace SpriteEditor
{
/**
 * Holds the data necessary for editing and saving a floor sprite set.
 * Part of SpriteSetModel. 
 */
struct EditorWallSpriteSet {
    /** This sprite set's unique numeric identifier.
        Note: This ID may change when this sprite set is saved to the json. */
    Uint16 numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each sprite in this set.
        Walls require the 4 types of wall sprites that our modular wall 
        system uses. */
    std::array<int, Wall::Type::Count> spriteIDs{
        NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID};
};

} // namespace SpriteEditor
} // namespace AM
