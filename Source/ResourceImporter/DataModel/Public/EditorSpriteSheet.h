#pragma once

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
    /** The path to the sprite sheet image file, relative to the application's
        base directory. */
    std::string relPath{};

    /** The runtime IDs for each sprite in this sheet. */
    std::vector<int> spriteIDs{};
};

} // namespace ResourceImporter
} // namespace AM
