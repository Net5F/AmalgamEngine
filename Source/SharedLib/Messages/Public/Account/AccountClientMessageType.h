#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Messages sent between the account server and clients (ran by users).
 */
enum class AccountClientMessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet,

    // Client -> Server Messages
    RegisterRequest,
    LoginRequest,
    LogoutRequest,
    RecoverAccountRequest,
    ServiceTicketRequest,

    // Server -> Client Messages
    RegisterResponse,
    LoginResponse,
    LogoutResponse,
    RecoverAccountResponse,
    ServiceTicketResponse,
};

} // End namespace AM
