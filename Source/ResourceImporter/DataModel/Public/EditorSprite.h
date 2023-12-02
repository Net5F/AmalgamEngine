#pragma once

#include "BoundingBox.h"
#include "NullSpriteID.h"
#include <SDL_rect.h>
#include <string>

namespace AM
{
namespace ResourceImporter
{

/**
 * Holds the data necessary for editing and saving a sprite.
 * Part of SpriteModel. 
 */
struct EditorSprite {
    /** This sprite's unique numeric identifier. */
    int numericID{NULL_SPRITE_ID};

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

} // namespace ResourceImporter
} // namespace AM
