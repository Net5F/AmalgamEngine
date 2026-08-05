#pragma once

#include "AccountDefs.h"
#include "AccountMessageType.h"
#include <string>

namespace AM
{

/**
 * Sent by the server in response to a registration request.
 */
struct RegisterResponse {
    // The AccountMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::RegisterResponse};

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
void serialize(S& serializer, RegisterResponse& registerResponse)
{
    serializer.value1b(registerResponse.result);
    serializer.text1b(registerResponse.recoveryKey, RECOVERY_KEY_CHARACTERS);
}

} // End namespace AM
