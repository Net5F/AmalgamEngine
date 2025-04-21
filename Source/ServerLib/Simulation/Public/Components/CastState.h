#pragma once

#include "CastableID.h"
#include "CastInfo.h"
#include <SDL_stdinc.h>

namespace AM
{
namespace Server
{

/**
 * Tracks whether an entity is currently casting a Castable.
 * 
 * To optimize performance, this component will only be present on an entity if 
 * a cast is currently ongoing. It gets removed when the cast ends.
 *
 * Note: This is server-only because the client needs its own version to track 
 *       additional data.
 */
struct CastState {
    /** The current cast's info. */
    CastInfo castInfo{};

    /** The tick that this cast will finish on. If 0, this cast hasn't been 
        processed for the first time yet. */
    Uint32 endTick{};
};

} // namespace Server
} // namespace AM
