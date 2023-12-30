#pragma once

#include "MovementState.h"
#include <SDL_stdinc.h>

namespace AM
{
namespace Client
{
/**
 * Holds updated movement state for the player entity.
 *
 * This isn't an actual message that gets sent by the server, instead it gets
 * split out when we receive a ComponentUpdate with the relevant data.
 */
struct PlayerMovementUpdate : public MovementState {
    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};
};

} // End namespace Client
} // End namespace AM
