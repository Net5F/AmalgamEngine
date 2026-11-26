#pragma once

#include "AccountClientMessageType.h"
#include "AccountDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by the client to ban an account, either permanently or for a duration.
 *
 * AccountServer checks the requester's role against the target's, applies the
 * ban, records it in the moderation log, and responds with a BanResponse. It
 * then sends an AccountBannedNotice to every service, so they can
 * disconnect the target's clients.
 */
struct BanRequest {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::BanRequest};

    /** This message holds secrets, so its buffers are wiped. */
    static constexpr bool IS_SENSITIVE{true};

    /** The longest ban that can be given, in seconds (10 years). For anything
        longer, use a permanent ban. */
    static constexpr Sint64 MAX_DURATION_S{60LL * 60 * 24 * 365 * 10};

    /** The client's current account session token. Identifies the
        requester. */
    SessionToken accountSessionToken{};

    /** The username of the account to ban. */
    std::string targetUsername{};

    /** How long the ban lasts, in seconds. 0 means permanent.
        Must be at most MAX_DURATION_S. */
    Sint64 durationS{0};

    /** (Optional) Why the ban was given. Recorded in the moderation log. */
    std::string reason{};
};

template<typename S>
void serialize(S& serializer, BanRequest& banRequest)
{
    serializer.container1b(banRequest.accountSessionToken.get());
    serializer.text1b(banRequest.targetUsername, USERNAME_MAX_LENGTH);
    serializer.value8b(banRequest.durationS);
    serializer.text1b(banRequest.reason, MODERATION_REASON_MAX_LENGTH);
}

} // End namespace AM
