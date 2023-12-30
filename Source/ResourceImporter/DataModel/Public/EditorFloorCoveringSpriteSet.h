#pragma once

#include "Rotation.h"
#include "NullSpriteID.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>

namespace AM
{
namespace ResourceImporter
{
/**
 * Holds the data necessary for editing and saving a floor covering sprite set.
 * Part of SpriteSetModel.
 */
struct EditorFloorCoveringSpriteSet {
    /** This sprite set's unique numeric identifier.
        Note: This ID may change when this sprite set is saved to the json. */
    Uint16 numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each sprite in this set.
        Floor coverings support 8 directions of rotation. At least 1 sprite
        must be set. If a direction isn't provided, it should be set to
        EMPTY_SPRITE_ID. */
    std::array<int, Rotation::Direction::Count> spriteIDs{
        NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID,
        NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID};
};

} // namespace ResourceImporter
} // namespace AM
