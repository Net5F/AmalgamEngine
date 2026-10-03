#pragma once

#include "AccountClientMessageType.h"
#include <string>

namespace AM
{

/**
 * Sent by the client to register an account.
 */
struct RegisterRequest {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::RegisterRequest};

    /** Used as a "we should never hit this" cap on username length. */
    static constexpr std::size_t USERNAME_MAX{100};
    /** Used as a "we should never hit this" cap on password length. */
    static constexpr std::size_t PASSWORD_MAX{100};

    /** The desired username. */
    std::string username{};

    // TODO: Need SecureString, secure fill
    /** The desired password in plaintext. */
    std::string password{};
};

template<typename S>
void serialize(S& serializer, RegisterRequest& registerRequest)
{
    serializer.text1b(registerRequest.username, RegisterRequest::USERNAME_MAX);
    serializer.text1b(registerRequest.password, RegisterRequest::PASSWORD_MAX);
}

} // End namespace AM
