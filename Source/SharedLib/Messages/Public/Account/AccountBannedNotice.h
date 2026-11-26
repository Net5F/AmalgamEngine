#pragma once

#include "AccountServiceMessageType.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by AccountServer to every connected service when an account is
 * banned. Services should disconnect the account's clients. Its sessions have
 * already been revoked, so it won't be able to reconnect.
 *
 * Unlike our other AccountServer messages, this isn't a response to a request.
 */
struct AccountBannedNotice {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::AccountBannedNotice};

    /** The account that was banned. */
    Sint64 accountID{0};
};

template<typename S>
void serialize(S& serializer, AccountBannedNotice& notice)
{
    serializer.value8b(notice.accountID);
}

} // End namespace AM
