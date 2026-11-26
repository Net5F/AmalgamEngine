#pragma once

#include "AccountClientMessageType.h"
#include "AccountDefs.h"
#include <string>

namespace AM
{

/**
 * Sent by the client to lift an account's active ban.
 *
 * AccountServer checks the requester's role against the target's, lifts the
 * ban, records it in the moderation log, and responds with an UnbanResponse.
 */
struct UnbanRequest {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::UnbanRequest};

    /** This message holds secrets, so its buffers are wiped. */
    static constexpr bool IS_SENSITIVE{true};

    /** The client's current account session token. Identifies the
        requester. */
    SessionToken accountSessionToken{};

    /** The username of the account to unban. */
    std::string targetUsername{};

    /** (Optional) Why the ban was lifted. Recorded in the moderation log. */
    std::string reason{};
};

template<typename S>
void serialize(S& serializer, UnbanRequest& unbanRequest)
{
    serializer.container1b(unbanRequest.accountSessionToken.get());
    serializer.text1b(unbanRequest.targetUsername, USERNAME_MAX_LENGTH);
    serializer.text1b(unbanRequest.reason, MODERATION_REASON_MAX_LENGTH);
}

} // End namespace AM
