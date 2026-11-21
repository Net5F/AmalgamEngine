#pragma once

#include "AccountClientMessageType.h"
#include "AccountDefs.h"
#include "AccountRoleDefs.h"
#include "ModerationDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by the client to ban, unban, or change the role of another account.
 *
 * AccountServer checks the requester's role against the target's, applies the
 * action, records it in the moderation log, and responds with a
 * AccountModerationResponse. If the action affects connected clients, it also
 * sends an AccountModeratedNotice to every service.
 *
 * For world-level actions (e.g. teleporting), see WorldModerationRequest.
 */
struct AccountModerationRequest {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::AccountModerationRequest};

    /** This message holds secrets, so its buffers are wiped. */
    static constexpr bool IS_SENSITIVE{true};

    /** The client's current account session token. Identifies the
        requester. */
    SessionToken accountSessionToken{};

    AccountModerationAction action{AccountModerationAction::Ban};

    /** The username of the account to act on. */
    std::string targetUsername{};

    /** (SetRole only) The ID of the role to give the target. */
    std::string roleID{};

    /** (Ban only) How long the ban lasts, in seconds. 0 means permanent.
        Must be at most MAX_BAN_DURATION_S. */
    Sint64 durationS{0};

    /** (Optional) Why the action was taken. Recorded in the moderation log. */
    std::string reason{};
};

template<typename S>
void serialize(S& serializer, AccountModerationRequest& moderationRequest)
{
    serializer.container1b(moderationRequest.accountSessionToken.get());
    serializer.value1b(moderationRequest.action);
    serializer.text1b(moderationRequest.targetUsername, USERNAME_MAX_LENGTH);
    serializer.text1b(moderationRequest.roleID, ACCOUNT_ROLE_ID_MAX_LENGTH);
    serializer.value8b(moderationRequest.durationS);
    serializer.text1b(moderationRequest.reason, MODERATION_REASON_MAX_LENGTH);
}

} // End namespace AM
