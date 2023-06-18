#pragma once

#include "BoundingBox.h"
#include "entt/core/hashed_string.hpp"
#include <SDL_rect.h>
#include <SDL_stdinc.h>

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

    /** If true, this sprite's modelBounds will be used in collision checks.
        Most sprites will want collision enabled, but things like floors and 
        carpets usually don't need collision. */
    bool collisionEnabled{false};

    /** Model-space bounding box. Defines the sprite's 3D volume.
        Used in hit testing for user mouse events, and for collision checks (
        if collisionEnabled). */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace SpriteEditor
} // namespace AM
