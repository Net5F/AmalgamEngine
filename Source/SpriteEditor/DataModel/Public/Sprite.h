#pragma once

#include "BoundingBox.h"
#include <SDL_rect.h>
#include <SDL_stdinc.h>
#include <string>
#include <vector>

namespace AM
{
namespace SpriteEditor
{
struct SpriteSheet;

/**
 * Holds the data for a single sprite.
 * Used as part of SpriteEditor's data model.
 */
struct Sprite {
public:
    /** The unique relPath of the sprite sheet that this sprite is from. */
    std::string parentSpriteSheetPath{""};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** UV position and size in texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};

    /** How much this sprite should be offset in the Y direction to line up
        with its tile. Used to support tall tiles for the iso depth effect. */
    int yOffset{0};

    /** Model-space bounding boxes. Defines the sprite's 3D volume. */
    std::vector<BoundingBox> modelBounds;
};

} // namespace SpriteEditor
} // namespace AM
