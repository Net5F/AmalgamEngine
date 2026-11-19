#pragma once

#include "AccountClientMessageType.h"
#include "AccountDefs.h"
#include "AccountRoleDef.h"
#include "ModerationDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by AccountServer in response to an AccountModerationRequest.
 */
struct AccountModerationResponse {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::AccountModerationResponse};

    /** The action that was requested. */
    AccountModerationAction action{AccountModerationAction::Ban};

    ModerationResult result{ModerationResult::InternalError};

    /** The target's username. If result == Success, this is the username as
        it was registered (which may differ in case from the request). */
    std::string targetUsername{};

    /** (SetRole only) The ID of the role that was requested. */
    std::string roleID{};

    /** (Ban only) The ban's duration, in seconds. 0 means permanent. */
    Sint64 durationS{0};
};

template<typename S>
void serialize(S& serializer, AccountModerationResponse& moderationResponse)
{
    serializer.value1b(moderationResponse.action);
    serializer.value1b(moderationResponse.result);
    serializer.text1b(moderationResponse.targetUsername, USERNAME_MAX_LENGTH);
    serializer.text1b(moderationResponse.roleID, ACCOUNT_ROLE_ID_MAX_LENGTH);
    serializer.value8b(moderationResponse.durationS);
}

} // End namespace AM
