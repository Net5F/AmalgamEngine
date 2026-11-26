#pragma once

#include "AccountClientMessageType.h"
#include "AccountDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by AccountServer in response to a BanRequest.
 */
struct BanResponse {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::BanResponse};

    enum Result : Uint8 {
        Success,
        /** The requester lacks the Ban permission, or doesn't outrank the
            target. */
        NotPermitted,
        /** The requester's session is invalid or expired. */
        InvalidSession,
        /** No account has the given username. */
        TargetNotFound,
        /** The request couldn't be processed (e.g. the server couldn't be
            reached). */
        InternalError
    };
    Result result{Result::InternalError};

    /** The target's username, as given in the request. If result == Success,
        this is the username as it was registered (which may differ in case
        from the request). */
    std::string targetUsername{};

    /** The ban's duration, in seconds. 0 means permanent. */
    Sint64 durationS{0};
};

template<typename S>
void serialize(S& serializer, BanResponse& banResponse)
{
    serializer.value1b(banResponse.result);
    serializer.text1b(banResponse.targetUsername, USERNAME_MAX_LENGTH);
    serializer.value8b(banResponse.durationS);
}

} // End namespace AM
