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

    /** The static data for all sprites in this sheet. */
    std::vector<Sprite> sprites;
};

} // namespace SpriteEditor
} // namespace AM
