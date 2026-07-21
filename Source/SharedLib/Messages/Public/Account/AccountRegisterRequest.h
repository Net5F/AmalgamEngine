#pragma once

#include "AccountMessageType.h"
#include <string>

namespace AM
{

/**
 * Sent by the client to register an account.
 */
struct AccountRegisterRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::AccountRegisterRequest};

    /** Used as a "we should never hit this" cap on username length. */
    static constexpr std::size_t USERNAME_MAX{100};
    /** Used as a "we should never hit this" cap on password length. */
    static constexpr std::size_t PASSWORD_MAX{100};

    /** The desired username. */
    std::string username{};

    // TODO: Need SecureString, secure fill
    /** The desired password in plaintext.
        Note: We send this over a secure connection to avoid being 
              compromised. */
    std::string password{};
};

template<typename S>
void serialize(S& serializer, AccountRegisterRequest& accountRegisterRequest)
{
    serializer.text1b(accountRegisterRequest.username,
                      AccountRegisterRequest::USERNAME_MAX);
    serializer.text1b(accountRegisterRequest.password,
                      AccountRegisterRequest::PASSWORD_MAX);
}

} // End namespace AM
