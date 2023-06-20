#pragma once

#include "Wall.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>

namespace AM
{
namespace SpriteEditor
{
/**
 * Holds the data necessary for editing and saving a floor sprite set.
 * Part of SpriteDataModel. 
 */
struct EditorWallSpriteSet {
    /** This sprite set's unique numeric identifier.
        Note: This ID may change when this sprite set is saved to the json. */
    Uint16 numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The runtime IDs for each sprite in this set.
        Walls require the 4 types of wall sprites that our modular wall 
        system uses. */
    std::array<int, Wall::Type::Count> spriteIDs;
};

} // namespace SpriteEditor
} // namespace AM
