#pragma once

namespace AM
{
namespace Client
{
/**
 * Represents an entity's animation state.
 *
 * Note: This is used by Renderer/WorldSpriteSorter. Usually only sim 
 *       classes are allowed to update components, but it's convenient here.
 */
struct AnimationState
{
    /** A timestamp of when the current animation was started. */
    double animationStartTime{};

    /** If true, a new animation has begun and animationStartTime needs to be  
        set when its first frame is rendered. */
    bool setStartTime{true};
};

} // namespace Client
} // namespace AM
