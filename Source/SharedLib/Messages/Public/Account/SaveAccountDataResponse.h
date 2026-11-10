#pragma once

#include "AccountServiceMessageType.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by AccountServer after attempting to save a player entity's
 * AccountData.
 */
struct SaveAccountDataResponse {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::SaveAccountDataResponse};

    /** The requestID of the SaveAccountDataRequest that this responds to. */
    Uint32 requestID{0};

    /**
     * InvalidAccount: The account doesn't exist. Retrying won't help.
     * InternalError: The save failed (or AccountServer couldn't be reached).
     *                The save should be retried.
     */
    enum Result : Uint8 { Success, InvalidAccount, InternalError };
    Result result{Result::InternalError};
};

template<typename S>
void serialize(S& serializer, SaveAccountDataResponse& saveAccountDataResponse)
{
    serializer.value4b(saveAccountDataResponse.requestID);
    serializer.value1b(saveAccountDataResponse.result);
}

} // End namespace AM
