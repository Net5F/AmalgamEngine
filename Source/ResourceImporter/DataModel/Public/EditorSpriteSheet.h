#pragma once

#include "SpriteSheetID.h"
#include <SDL_rect.h>
#include <string>
#include <vector>

namespace AM
{
namespace ResourceImporter
{
/**
 * Holds the data necessary for editing and saving a sprite sheet.
 * Part of SpriteModel.
 */
struct EditorSpriteSheet {
    /** This sprite sheet's unique numeric identifier. */
    SpriteSheetID numericID{NULL_SPRITE_SHEET_ID};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The width of the generated sprite sheet texture. */
    int textureWidth{0};
    /** The height of the generated sprite sheet texture. */
    int textureHeight{0};

    /** The runtime IDs for each sprite in this sheet. */
    std::vector<int> spriteIDs{};
};

} // namespace ResourceImporter
} // namespace AM
