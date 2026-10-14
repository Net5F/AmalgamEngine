#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Messages sent between the chat server and our other internal services
 * (world server).
 */
enum class ChatServiceMessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet,

    // World Server -> Chat Server
    PlayerPositionsUpdate,
    PlayersLeftWorld,
    ChatServiceHeartbeat,

    // Chat Server -> World Server
    ChatServiceHeartbeatResponse,
};

} // End namespace AM
