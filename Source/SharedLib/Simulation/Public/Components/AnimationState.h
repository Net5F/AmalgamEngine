#pragma once

#include "SpriteSets.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Represents an entity's animation state.
 */
struct AnimationState {
    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    // Note: These will all change to "animationSetType", "animationSetID", etc
    //       when we implement animations.
    /** The type of sprite set that this entity uses. */
    SpriteSet::Type spriteSetType{SpriteSet::Type::None};

    /** The numeric ID of this entity's sprite set. */
    Uint16 spriteSetID{0};

    /** The index within spriteSet.sprites of this entity's current sprite. */
    Uint8 spriteIndex{0};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    // TODO: When we implement animations, we should add frameRate, frameIndex,
    //       etc (current animation state).
};

template<typename S>
void serialize(S& serializer, AnimationState& animationState)
{
    serializer.value1b(animationState.spriteSetType);
    serializer.value2b(animationState.spriteSetID);
    serializer.value1b(animationState.spriteIndex);
}

} // namespace AM
