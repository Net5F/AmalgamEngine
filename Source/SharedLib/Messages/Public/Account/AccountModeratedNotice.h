#pragma once

#include "AccountServiceMessageType.h"
#include "ModerationDefs.h"
#include "AccountRoleDef.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by AccountServer to every connected service when a moderation action
 * affects an account's connected clients.
 *
 * Ban: Disconnect the account's clients. Its sessions have already been
 *      revoked, so it won't be able to reconnect.
 * SetRole: Update the account's role.
 *
 * Unlike our other AccountServer messages, this isn't a response to a request.
 */
struct AccountModeratedNotice {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::AccountModeratedNotice};

    /** The account that was acted on. */
    Sint64 accountID{0};

    /** Ban or SetRole. */
    AccountModerationAction action{AccountModerationAction::Ban};

    /** (SetRole only) The account's new role ID. */
    std::string roleID{};
};

template<typename S>
void serialize(S& serializer, AccountModeratedNotice& notice)
{
    serializer.value8b(notice.accountID);
    serializer.value1b(notice.action);
    serializer.text1b(notice.roleID, ACCOUNT_ROLE_ID_MAX_LENGTH);
}

} // End namespace AM
