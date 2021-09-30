#pragma once

#include <SDL2/SDL_stdinc.h>

namespace AM
{

/**
 * The types of messages that we send across the network.
 *
 * For descriptions, see their definitions in Shared/Messages/Public.
 */
enum class MessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet = 0,
    ExplicitConfirmation = 1,
    Heartbeat = 2,
    ConnectionResponse = 3,
    EntityUpdate = 4,
    ClientInput = 5,
    UpdateChunks = 6,
};

} // End namespace AM
