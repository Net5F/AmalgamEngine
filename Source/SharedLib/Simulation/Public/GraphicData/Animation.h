#pragma once

#include "AnimationID.h"
#include "SpriteID.h"
#include "BoundingBox.h"
#include <string>
#include <vector>
#include <functional>

namespace AM
{
struct Sprite;

/**
 * Holds the data for a single animation from ResourceData.json.
 *
 * World position should be read from an associated Position component (if
 * this animation is attached to an entity), or derived from an associated Tile.
 */
struct Animation {
    /** Unique display name, shown in the UI.  */
    std::string displayName{"Null"};

    /** The animation's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"null"};

    /** This animation's unique numeric identifier.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when ResourceData.json is
        modified. */
    AnimationID numericID{NULL_ANIMATION_ID};

    /** How long this animation is, in frames. */
    Uint8 frameCount{0};

    /** This animation's framerate (frames per second). */
    Uint8 fps{0};

    struct Frame
    {
        Uint8 frameNumber{0};
        std::reference_wrapper<const Sprite> sprite;
    };
    /** The frames of this animation, ordered by ascending frameNumber.
        Note: There will always be at least 1 frame present. If this animation 
              was left empty in the editor, it will be given the null sprite. */
    std::vector<Frame> frames;

    /** If true, this animation's modelBounds will be used in collision checks.
        Most animations will want collision enabled, but things like floors and
        carpets usually don't need collision. */
    bool collisionEnabled{false};

    /** This animation's model-space bounding volume.
        Note: Tiles use these bounds, but entities use the bounds defined by 
              their Collision component. */
    BoundingBox modelBounds{};

    /**
     * Returns the sprite that should be displayed at the given animation time, 
     * or nullptr if this animation has no frames.
     */
    const Sprite& getSpriteAtTime(double animationTime) const;
};

} // namespace AM
