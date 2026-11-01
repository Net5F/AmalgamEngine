#pragma once

#include "AccountServiceMessageType.h"
#include "AccountData.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by the world server to save a client entity's AccountData.
 *
 * Sent periodically while the entity is in the world, and once more when it's
 * removed.
 */
struct SaveAccountDataRequest {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::SaveAccountDataRequest};

    /** Chosen by the requesting service. Echoed back in the response so
        the service can match it to this request. */
    Uint32 requestID{0};

    /** The account that the data belongs to. */
    Sint64 accountID{0};

    /** The data to save. Overwrites any existing data. */
    AccountData accountData{};
};

template<typename S>
void serialize(S& serializer, SaveAccountDataRequest& saveAccountDataRequest)
{
    serializer.value4b(saveAccountDataRequest.requestID);
    serializer.value8b(saveAccountDataRequest.accountID);
    serializer.object(saveAccountDataRequest.accountData);
}

} // End namespace AM
