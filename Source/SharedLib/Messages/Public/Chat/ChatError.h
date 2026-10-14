#pragma once

#include "ChatClientMessageType.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by the chat server when it rejects a ChatSendRequest.
 */
struct ChatError {
    static constexpr ChatClientMessageType MESSAGE_TYPE{
        ChatClientMessageType::ChatError};

    enum Type : Uint8 {
        /** The client is sending messages too quickly. */
        RateLimited,
        /** The message's text was empty, too long, or contained disallowed
            characters. */
        InvalidText,
        /** Local chat requires the sender to be in the world. */
        NotInWorld
    };
    Type type{Type::InvalidText};
};

template<typename S>
void serialize(S& serializer, ChatError& chatError)
{
    serializer.value1b(chatError.type);
}

} // End namespace AM
