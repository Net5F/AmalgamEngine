#pragma once

#include "AccountDefs.h"
#include "AccountMessageType.h"
#include <array>

namespace AM
{

/**
 * Sent by the client to revoke its current account session.
 */
struct LogoutRequest {
    // The AccountMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::LogoutRequest};

    /** The account session to revoke. */
    std::array<Uint8, SESSION_TOKEN_BYTES> sessionToken{};
};

template<typename S>
void serialize(S& serializer, LogoutRequest& logoutRequest)
{
    serializer.container1b(logoutRequest.sessionToken);
}

} // End namespace AM
