#pragma once

#include "EntityGraphicType.h"

namespace AM
{
namespace Client
{
/**
 * Holds client-only graphic-related state.
 *
 * Note: This is modified by both the sim and Renderer/WorldSpriteSorter.
 *       Usually only sim classes are allowed to update components, but it's 
 *       convenient here.
 */
struct ClientGraphicState
{
    /** The entity's current graphic type.
        The systems that set this will make sure it's always a valid slot 
        within the entity's graphic set. */
    EntityGraphicType graphicType{};

    /** A timestamp of when the current animation was started. */
    double animationStartTime{};

    /** If true, a new animation has begun and animationStartTime needs to be  
        reset when its first frame is rendered. */
    bool setStartTime{true};
};

} // namespace Client
} // namespace AM
