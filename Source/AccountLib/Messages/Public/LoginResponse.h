#pragma once

#include "AccountDefs.h"
#include "AccountMessageType.h"
#include <array>

namespace AM
{

/**
 * Sent by the server in response to a login request.
 */
struct LoginResponse {
    // The AccountMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::LoginResponse};

    enum Result : Uint8 {
        Success,
        InvalidAccountDetails,
        RateLimited,
        InternalError
    };
    Result result{Result::InternalError};

    /** If result == success, this is the ID of the authenticated account. */
    Sint64 accountID{0};

    /** If result == success, this is a valid session token. */
    std::array<Uint8, SESSION_TOKEN_BYTES> sessionToken{};

    /** If result == success, this is the Unix timestamp at which the session
        will expire due to inactivity. */
    Sint64 idleExpiresAt{0};

    /** If result == success, this is the Unix timestamp at which the session
        will expire regardless of activity. */
    Sint64 absoluteExpiresAt{0};
};

template<typename S>
void serialize(S& serializer, LoginResponse& loginResponse)
{
    serializer.value1b(loginResponse.result);
    serializer.value8b(loginResponse.accountID);
    serializer.container1b(loginResponse.sessionToken);
    serializer.value8b(loginResponse.idleExpiresAt);
    serializer.value8b(loginResponse.absoluteExpiresAt);
}

} // End namespace AM
