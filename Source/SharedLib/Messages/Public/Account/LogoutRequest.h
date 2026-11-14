#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"

namespace AM
{

/**
 * Sent by the client to revoke its current account session.
 */
struct LogoutRequest {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::LogoutRequest};

    /** The account session to revoke. */
    SessionToken sessionToken{};
};

template<typename S>
void serialize(S& serializer, LogoutRequest& logoutRequest)
{
    serializer.container1b(logoutRequest.sessionToken.get());
}

} // End namespace AM
