#pragma once

#include "AccountMessageType.h"
#include <string>

namespace AM
{

/**
 * Sent by the client to log into an account.
 */
struct LoginRequest {
    // The AccountMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountMessageType MESSAGE_TYPE{
        AccountMessageType::LoginRequest};

    /** Used as a "we should never hit this" cap on username length. */
    static constexpr std::size_t USERNAME_MAX{100};
    /** Used as a "we should never hit this" cap on password length. */
    static constexpr std::size_t PASSWORD_MAX{100};

    /** The account's username. */
    std::string username{};

    // TODO: Need SecureString, secure fill
    /** The account's password in plaintext. */
    std::string password{};
};

template<typename S>
void serialize(S& serializer, LoginRequest& loginRequest)
{
    serializer.text1b(loginRequest.username, LoginRequest::USERNAME_MAX);
    serializer.text1b(loginRequest.password, LoginRequest::PASSWORD_MAX);
}

} // End namespace AM
