#pragma once

#include "SpriteID.h"
#include "SpriteSheetID.h"
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
 * Holds the data necessary for editing and saving a sprite.
 * Part of SpriteModel.
 */
struct EditorSprite {
    /** This sprite's unique numeric identifier. */
    SpriteID numericID{NULL_SPRITE_ID};

    // Note: We don't need to store the sprite sheet ID, since sprite sheets 
    //       know which sprites they own (both in memory and in json).

    /** The relPath to the individual image file for this sprite.
        Note: This path isn't used by the engine, but we need to save it to 
              the json for use by the editor (we use it when building sprite 
              sheets). */
    std::string imagePath{""};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** This sprite's actual-space UV position and size within its parent 
        sprite sheet texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};

    /** The actual-space point within the sprite where the "stage" starts.
        The "stage" is the coordinate space that we overlay onto the sprite 
        image. */
    SDL_Point stageOrigin{0, 0};

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

    /** If true, this sprite will have its alpha premultiplied. */
    bool premultiplyAlpha{false};

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
