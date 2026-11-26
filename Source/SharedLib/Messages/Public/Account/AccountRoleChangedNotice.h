#pragma once

#include "AccountServiceMessageType.h"
#include "AccountRoleDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by AccountServer to every connected service when an account's role is
 * changed. Services should update the account's role.
 *
 * Unlike our other AccountServer messages, this isn't a response to a request.
 */
struct AccountRoleChangedNotice {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::AccountRoleChangedNotice};

    /** The account whose role was changed. */
    Sint64 accountID{0};

    /** The account's new role ID. */
    std::string roleID{};
};

template<typename S>
void serialize(S& serializer, AccountRoleChangedNotice& notice)
{
    serializer.value8b(notice.accountID);
    serializer.text1b(notice.roleID, ACCOUNT_ROLE_ID_MAX_LENGTH);
}

} // End namespace AM
