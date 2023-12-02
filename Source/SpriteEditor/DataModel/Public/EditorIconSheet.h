#pragma once

#include "IconID.h"
#include <SDL_stdinc.h>
#include <string>
#include <vector>

namespace AM
{
namespace SpriteEditor
{
/**
 * Holds the data necessary for editing and saving an icon sheet.
 * Part of IconModel. 
 */
struct EditorIconSheet {
    /** The path to the icon sheet image file, relative to the application's
        base directory. */
    std::string relPath{};

    /** The runtime IDs for each icon in this sheet. */
    std::vector<IconID> iconIDs{};
};

} // namespace SpriteEditor
} // namespace AM
