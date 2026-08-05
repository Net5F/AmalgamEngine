#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * The types of messages exchanged across Account Server connections.
 *
 * For message descriptions, see their definitions in Shared/Messages/Public.
 */
enum class AccountMessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet,

    // Client -> Server Messages
    RegisterRequest,
    LoginRequest,
    LogoutRequest,
    RequestWorldTicket,

    // Trusted World Server -> Account Server Messages
    ConsumeWorldTicketRequest,

    // Server -> Client Messages
    RegisterResponse,
    LoginResponse,
    LogoutResponse,
    ServiceTicketIssued,

    // Account Server -> Trusted World Server Messages
    ConsumeWorldTicketResponse,
};

} // End namespace AM
