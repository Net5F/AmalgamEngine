#pragma once

#include "AccountMessageType.h"
#include <string>

namespace AM
{

/**
 * Sent by the server in response to a registration request.
 */
struct AccountRegisterResponse {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::AccountRegisterResponse};

    /** Used as a "we should never hit this" cap on recovery key length. */
    static constexpr std::size_t RECOVERY_KEY_MAX{100};

    enum Result : Uint8 {
        Success,
        InvalidUsername,
        InvalidPassword,
        UsernameUnavailable,
        RateLimited,
        RegistrationDisabled,
        InternalError
    };
    Result result{Result::InternalError};

    // TODO: Need SecureString, secure fill
    /** If result == success, this is the recovery key for the account. */
    std::string recoveryKey{};
};

template<typename S>
void serialize(S& serializer, AccountRegisterResponse& accountRegisterResponse)
{
    serializer.value1b(accountRegisterResponse.result);
    serializer.text1b(accountRegisterResponse.recoveryKey,
                      AccountRegisterResponse::RECOVERY_KEY_MAX);
}

} // End namespace AM
