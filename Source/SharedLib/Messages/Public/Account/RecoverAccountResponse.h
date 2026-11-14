#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"

namespace AM
{

/**
 * Sent by the server in response to an account recovery request.
 */
struct RecoverAccountResponse {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::RecoverAccountResponse};

    /** This message holds secrets, so its buffers are wiped. */
    static constexpr bool IS_SENSITIVE{true};

    enum Result : Uint8 {
        Success,
        /** The username or recovery key was incorrect, or the account is
            unavailable. */
        InvalidAccountDetails,
        InvalidPassword,
        RateLimited,
        InternalError
    };
    Result result{Result::InternalError};

    /** If result == success, this is the account's new recovery key.
        Recovery keys are single-use, so the old key is no longer valid. */
    RecoveryKey recoveryKey{};
};

template<typename S>
void serialize(S& serializer, RecoverAccountResponse& recoverAccountResponse)
{
    serializer.value1b(recoverAccountResponse.result);
    serializer.text1b(recoverAccountResponse.recoveryKey,
                      RECOVERY_KEY_CHARACTERS);
}

} // End namespace AM
