#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Messages sent between the chat server and clients.
 */
enum class ChatClientMessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet,

    // Client -> Chat Server
    ChatConnectionRequest,
    ChatSendRequest,
    ChatHeartbeat,

    // Chat Server -> Client
    ChatConnectionResponse,
    ChatMessage,
    ChatError,
    ChatHeartbeatResponse,
};

} // End namespace AM
