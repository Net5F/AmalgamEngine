#pragma once

#include "SpriteSheetID.h"
#include "SpriteID.h"
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
    /** This sprite sheet's unique numeric identifier.
        Note: Sprite sheet numeric IDs aren't saved to the json or used by 
              the engine, so we just generate new ones each time the 
              importer is ran. */
    SpriteSheetID numericID{NULL_SPRITE_SHEET_ID};

    /** Unique display name. Shown in the UI, and used as the name of the 
        exported sprite sheet image file. */
    std::string displayName{""};

    /** The width of the generated sprite sheet texture. */
    int textureWidth{0};
    /** The height of the generated sprite sheet texture. */
    int textureHeight{0};

    /** The numeric IDs for each sprite in this sheet. */
    std::vector<SpriteID> spriteIDs{};
};

} // namespace ResourceImporter
} // namespace AM
