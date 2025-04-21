#pragma once

#include "CastableID.h"
#include "CastInfo.h"
#include <SDL_stdinc.h>

namespace AM
{
namespace Client
{

/**
 * Tracks whether an entity is currently casting a Castable.
 * 
 * To match the behavior of Server::CastState, this component will only be 
 * present on an entity if a cast is currently ongoing. It gets removed when 
 * the cast ends.
 */
struct ClientCastState {
    /** The current cast's info. */
    CastInfo castInfo{};

    enum class State {
        /** A cast is ongoing. */
        Casting,
        /** The cast has successfully completed and the "cast complete" 
            graphic should be shown. */
        CastComplete
    };
    /** The casting state that the entity is currently in. */
    State state{};

    /** The tick that the current state will finish on. If 0, this cast hasn't 
        been processed for the first time yet. */
    Uint32 endTick{0};
};

} // namespace Client
} // namespace AM
