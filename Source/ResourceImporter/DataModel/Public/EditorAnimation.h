#pragma once

#include "SpriteID.h"
#include "BoundingBoxID.h"
#include "BoundingBox.h"
#include <SDL_rect.h>
#include <string>

namespace AM
{
namespace ResourceImporter
{

class BoundingBoxModel;

/**
 * Holds the data necessary for editing and saving an animation.
 * Part of AnimationModel.
 */
struct EditorAnimation {
    /** This sprite's unique numeric identifier. */
    SpriteID numericID{NULL_SPRITE_ID};

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

    /** If non-null, this is the ID of this sprite's model-space bounding box.
        Defines the sprite's 3D volume.
        Used in hit testing for user mouse events, and for collision checks (
        if collisionEnabled). */
    BoundingBoxID modelBoundsID{NULL_BOUNDING_BOX_ID};

    /** If modelBoundsID is null, this is the sprite's custom model-space 
        bounding box. */
    BoundingBox customModelBounds{};

    /**
     * Returns this sprite's model-space bounding box.
     *
     * If modelBoundsID is non-null, returns the associated bounding box. Else, 
     * returns customModelBounds.
     */
    const BoundingBox&
        getModelBounds(const BoundingBoxModel& boundingBoxModel) const;
};

} // namespace ResourceImporter
} // namespace AM
