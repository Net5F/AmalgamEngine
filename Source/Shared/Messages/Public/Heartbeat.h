#pragma once

#include "SDL_stdinc.h"
#include "InputComponent.h"

namespace AM
{
/**
 * Heartbeat is sent from either side to show that a tick was processed but
 * no data needed to be sent.
 */
struct Heartbeat {
    /** The tick that this heartbeat was processed on. */
    Uint32 tickNum = 0;
};

template<typename S>
void serialize(S& serializer, Heartbeat& heartbeat)
{
    serializer.value4b(heartbeat.tickNum);
}

} // End namespace AM
