#pragma once

#include "MessageType.h"
#include <SDL2/SDL_stdinc.h>

namespace AM
{
/**
 * A heartbeat is sent from either side to show that a tick was processed but
 * no data needed to be sent.
 */
struct Heartbeat {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::Heartbeat;

    /** The tick that this heartbeat was processed on. */
    Uint32 tickNum{0};
};

template<typename S>
void serialize(S& serializer, Heartbeat& heartbeat)
{
    serializer.value4b(heartbeat.tickNum);
}

} // End namespace AM
