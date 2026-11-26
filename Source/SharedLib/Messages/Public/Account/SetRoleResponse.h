#pragma once

#include "AccountClientMessageType.h"
#include "AccountDefs.h"
#include "AccountRoleDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by AccountServer in response to a SetRoleRequest.
 */
struct SetRoleResponse {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::SetRoleResponse};

    enum Result : Uint8 {
        Success,
        /** The requester lacks the SetRole permission, or doesn't outrank
            the target or the role being assigned. */
        NotPermitted,
        /** The requester's session is invalid or expired. */
        InvalidSession,
        /** No account has the given username. */
        TargetNotFound,
        /** The given role doesn't exist. */
        InvalidRole,
        /** The request couldn't be processed (e.g. the server couldn't be
            reached). */
        InternalError
    };
    Result result{Result::InternalError};

    /** The target's username, as given in the request. If result == Success,
        this is the username as it was registered (which may differ in case
        from the request). */
    std::string targetUsername{};

    /** The ID of the role that was requested. */
    std::string roleID{};
};

template<typename S>
void serialize(S& serializer, SetRoleResponse& setRoleResponse)
{
    serializer.value1b(setRoleResponse.result);
    serializer.text1b(setRoleResponse.targetUsername, USERNAME_MAX_LENGTH);
    serializer.text1b(setRoleResponse.roleID, ACCOUNT_ROLE_ID_MAX_LENGTH);
}

} // End namespace AM
