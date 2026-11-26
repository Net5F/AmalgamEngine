#pragma once

#include "AccountClientMessageType.h"
#include "AccountDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by AccountServer in response to an UnbanRequest.
 */
struct UnbanResponse {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::UnbanResponse};

    enum Result : Uint8 {
        Success,
        /** The requester lacks the Ban permission, or doesn't outrank the
            target. */
        NotPermitted,
        /** The requester's session is invalid or expired. */
        InvalidSession,
        /** No account has the given username. */
        TargetNotFound,
        /** The target isn't banned. */
        NotBanned,
        /** The request couldn't be processed (e.g. the server couldn't be
            reached). */
        InternalError
    };
    Result result{Result::InternalError};

    /** The target's username, as given in the request. If result == Success,
        this is the username as it was registered (which may differ in case
        from the request). */
    std::string targetUsername{};
};

template<typename S>
void serialize(S& serializer, UnbanResponse& unbanResponse)
{
    serializer.value1b(unbanResponse.result);
    serializer.text1b(unbanResponse.targetUsername, USERNAME_MAX_LENGTH);
}

} // End namespace AM
