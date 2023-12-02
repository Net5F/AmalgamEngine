#pragma once

#include "IconID.h"
#include <SDL_stdinc.h>
#include <SDL_rect.h>
#include <string>

namespace AM
{
namespace ResourceImporter
{
/**
 * Holds the data necessary for editing and saving an icon.
 * Part of IconModel. 
 */
struct EditorIcon {
    /** This icon's unique numeric identifier.
        Note: This ID may change when this icon is saved to the json. */
    IconID numericID{0};

    /** The unique relPath of the icon sheet that this icon is from. */
    std::string parentIconSheetPath{""};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** UV position and size in texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};
};

} // namespace ResourceImporter
} // namespace AM
