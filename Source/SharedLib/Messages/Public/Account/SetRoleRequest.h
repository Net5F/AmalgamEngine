#pragma once

#include "AccountClientMessageType.h"
#include "AccountDefs.h"
#include "AccountRoleDefs.h"
#include <string>

namespace AM
{

/**
 * Sent by the client to change an account's role.
 *
 * AccountServer checks the requester's role against the target's and the
 * new role, changes the role, records it in the moderation log, and responds
 * with a SetRoleResponse. It then sends an AccountRoleChangedNotice to every
 * service, so they can update the target's AccountRole.
 */
struct SetRoleRequest {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::SetRoleRequest};

    /** This message holds secrets, so its buffers are wiped. */
    static constexpr bool IS_SENSITIVE{true};

    /** The client's current account session token. Identifies the
        requester. */
    SessionToken accountSessionToken{};

    /** The username of the account to change. */
    std::string targetUsername{};

    /** The ID of the role to give the target. */
    std::string roleID{};

    /** (Optional) Why the role was changed. Recorded in the moderation log. */
    std::string reason{};
};

template<typename S>
void serialize(S& serializer, SetRoleRequest& setRoleRequest)
{
    serializer.container1b(setRoleRequest.accountSessionToken.get());
    serializer.text1b(setRoleRequest.targetUsername, USERNAME_MAX_LENGTH);
    serializer.text1b(setRoleRequest.roleID, ACCOUNT_ROLE_ID_MAX_LENGTH);
    serializer.text1b(setRoleRequest.reason, MODERATION_REASON_MAX_LENGTH);
}

} // End namespace AM
