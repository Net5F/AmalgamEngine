#pragma once

#include "SpriteID.h"
#include "SpriteSetIDs.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>

namespace AM
{
namespace ResourceImporter
{
/**
 * Holds the data necessary for editing and saving a floor sprite set.
 * Part of SpriteSetModel.
 */
struct EditorFloorSpriteSet {
    /** This sprite set's unique numeric identifier.
        Note: This ID may change when this sprite set is saved to the json. */
    FloorSpriteSetID numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each sprite in this set.
        Floors currently only support 1 sprite, but support for "variations"
        may be added in the future. */
    std::array<SpriteID, 1> spriteIDs{NULL_SPRITE_ID};
};

} // namespace ResourceImporter
} // namespace AM
