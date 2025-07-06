#pragma once

#include "AnimationID.h"
#include "EditorSprite.h"
#include "BoundingBoxID.h"
#include "BoundingBox.h"
#include <SDL_rect.h>
#include <string>
#include <vector>
#include <functional>
#include <optional>

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
    /** This animation's unique numeric identifier. */
    AnimationID numericID{NULL_ANIMATION_ID};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** How long this animation is, in frames. */
    Uint8 frameCount{1};

    /** This animation's framerate (frames per second). */
    Uint8 fps{10};

    /** When the animation completes, it will loop to this frame and continue 
        playing. If == frameCount, no frames will be looped.
        Must always be <= frameCount. */
    Uint8 loopStartFrame{0};

    struct Frame
    {
        Uint8 frameNumber{0};
        // Note: This gets serialized in ResourceData.json as "spriteID".
        std::reference_wrapper<const EditorSprite> sprite;
    };
    /** The frames of this animation, ordered by ascending frameNumber.
        Only holds frames that actually contain a sprite.
        Note: There will always be at least 1 frame present. */
    std::vector<Frame> frames;

    /** If true, this animation's modelBounds will be used in collision checks.
        Most animations will want collision enabled, but things like floors and
        carpets usually don't need collision. */
    bool collisionEnabled{false};

    /** If non-null, this is the ID of this animation's model-space bounding box.
        Defines the animation's 3D volume.
        Used in hit testing for user mouse events, and for collision checks (
        if collisionEnabled). */
    BoundingBoxID modelBoundsID{NULL_BOUNDING_BOX_ID};

    /** If modelBoundsID is null, this is the animation's custom model-space 
        bounding box. */
    BoundingBox customModelBounds{};

    /** Only used for entities, during render sorting.
        When entities change animation, the new animation needs to line up with
        the old one so the entity doesn't look like it's teleporting around. 
        If non-null, this is the model-space point that should be aligned 
        with IdleSouth. */
    std::optional<Vector3> entityAlignmentAnchor{};

    /**
     * Returns this animation's model-space bounding box.
     *
     * If modelBoundsID is non-null, returns the associated bounding box. Else, 
     * returns customModelBounds.
     */
    const BoundingBox&
        getModelBounds(const BoundingBoxModel& boundingBoxModel) const;

    /**
     * Returns the sprite that should be displayed at the given animation time, 
     * or nullptr if this animation has no frames.
     */
    const EditorSprite* getSpriteAtTime(double animationTime) const;

    /**
     * Returns the sprite that should be displayed at the given frame.
     * If the given frame doesn't have a sprite, returns the closest sprite 
     * from a previous frame.
     * If there are no previous frames with a sprite, returns nullptr.
     */
    const EditorSprite* getSpriteAtFrame(Uint8 frameNumber) const;

    /**
     * Returns a vector that contains all the frames of this animation, 
     * including empty frames (as nullptr).
     *
     * This form is more convenient during modifications (add, swap, remove), 
     * but less convenient to iterate through.
     */
    std::vector<const EditorSprite*> getExpandedFrameVector() const;

    /**
     * Clears this animation and fills it to match the given vector.
     */
    void setFromExpandedFrameVector(
        const std::vector<const EditorSprite*>& expandedFrameVector);
};

} // namespace ResourceImporter
} // namespace AM
