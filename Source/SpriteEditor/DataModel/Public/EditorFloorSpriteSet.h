#pragma once

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
struct EditorFloorSpriteSet {
    /** This sprite set's unique numeric identifier.
        Note: This ID may change when this sprite set is saved to the json. */
    Uint16 numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The runtime IDs for each sprite in this set.
        Floors currently only support 1 sprite, but support for "variations" 
        may be added in the future. */
    std::array<int, 1> spriteIDs;
};

} // namespace SpriteEditor
} // namespace AM
