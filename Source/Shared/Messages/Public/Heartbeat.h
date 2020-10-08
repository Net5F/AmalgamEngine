#pragma once

#include "SDL_stdinc.h"
#include "InputComponent.h"

namespace AM
{
/**
 * This message is used by the client to send the server its sync information,
 * even if there was no other data queued to send.
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
