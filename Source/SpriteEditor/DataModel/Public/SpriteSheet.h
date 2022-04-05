#pragma once

#include "Sprite.h"
#include <string>
#include <vector>

namespace AM
{
namespace SpriteEditor
{
/**
 * Holds the path to a sprite sheet and all associated sprite data.
 */
struct SpriteSheet {
public:
    /** The path to the sprite sheet image file, relative to the application's
        base directory. */
    std::string relPath;

    /** The editor IDs for each sprite in this sheet.
        Refers to SpriteDataModel::spriteMap. */
    std::vector<unsigned int> spriteIDs;
};

} // namespace SpriteEditor
} // namespace AM
