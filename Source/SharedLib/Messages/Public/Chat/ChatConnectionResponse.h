#pragma once

#include "ChatClientMessageType.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by the chat server in response to a ChatConnectionRequest.
 *
 * If result != Success, the chat server will close the connection.
 */
struct ChatConnectionResponse {
    static constexpr ChatClientMessageType MESSAGE_TYPE{
        ChatClientMessageType::ChatConnectionResponse};

    enum Result : Uint8 {
        /** The client was authenticated and can now send and receive chat. */
        Success,
        /** The client's ticket was invalid, expired, or already used. */
        InvalidTicket,
        /** The server failed to validate the client's ticket (e.g. it
            couldn't reach the AccountServer). */
        InternalError
    };
    Result result{Result::InternalError};
};

template<typename S>
void serialize(S& serializer, ChatConnectionResponse& chatConnectionResponse)
{
    serializer.value1b(chatConnectionResponse.result);
}

} // End namespace AM
