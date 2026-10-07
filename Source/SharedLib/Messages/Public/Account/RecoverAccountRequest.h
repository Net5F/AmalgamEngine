#pragma once

#include "AccountDefs.h"
#include "AccountClientMessageType.h"
#include <string>

namespace AM
{

/**
 * Sent by the client to regain access to an account by using its recovery key
 * to set a new password.
 */
struct RecoverAccountRequest {
    // The message enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr AccountClientMessageType MESSAGE_TYPE{
        AccountClientMessageType::RecoverAccountRequest};

    /** Used as a "we should never hit this" cap on username length. */
    static constexpr std::size_t USERNAME_MAX{100};
    /** Used as a "we should never hit this" cap on password length. */
    static constexpr std::size_t PASSWORD_MAX{100};

    /** The account's username. */
    std::string username{};

    // TODO: Need SecureString, secure fill
    /** The account's current recovery key. */
    std::string recoveryKey{};

    // TODO: Need SecureString, secure fill
    /** The desired new password in plaintext. */
    std::string newPassword{};
};

template<typename S>
void serialize(S& serializer, RecoverAccountRequest& recoverAccountRequest)
{
    serializer.text1b(recoverAccountRequest.username,
                      RecoverAccountRequest::USERNAME_MAX);
    serializer.text1b(recoverAccountRequest.recoveryKey,
                      RECOVERY_KEY_CHARACTERS);
    serializer.text1b(recoverAccountRequest.newPassword,
                      RecoverAccountRequest::PASSWORD_MAX);
}

} // End namespace AM
