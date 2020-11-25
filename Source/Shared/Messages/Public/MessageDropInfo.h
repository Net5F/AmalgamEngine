#pragma once

#include "SDL_stdinc.h"

namespace AM
{
/**
 * This message is used by the server to inform the client that an input
 * message that it sent was dropped.
 */
struct MessageDropInfo {
    /** The tick that the dropped message was intended for. */
    Uint32 tickNum = 0;
};

template<typename S>
void serialize(S& serializer, MessageDropInfo& messageDropInfo)
{
    serializer.value4b(messageDropInfo.tickNum);
}

} // End namespace AM
