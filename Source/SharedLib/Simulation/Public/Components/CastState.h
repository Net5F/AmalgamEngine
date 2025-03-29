#pragma once

#include "CastableID.h"
#include "CastInfo.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * Tracks whether an entity is currently casting a Castable.
 * 
 * This component will only be present on an entity if a cast is currently 
 * ongoing. It gets removed when the cast ends.
 *
 * Note: We manually replicate this component. If we auto-replicated, all 
 *       destructions would be replicated. By handling it manually, we can get 
 *       away with only sending CastStarted and CastCanceled (a successful cast 
 *       can be assumed when there's no cancelation before endTick).
 */
struct CastState {
    /** The current cast's info. */
    CastInfo castInfo{};

    /** The tick that this cast will finish on. If 0, this cast hasn't been 
        processed for the first time yet. */
    Uint32 endTick{};
};

} // namespace AM
