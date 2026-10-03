#pragma once

#include <SDL3/SDL_stdinc.h>
#include <cstddef>

/**
 * This file contains shared account type definitions that should be consistent
 * between the account server and client.
 */
namespace AM
{
//--------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------
/** Username min/max length (arbitrarily chosen). */
inline constexpr int USERNAME_MIN_LENGTH{3};
inline constexpr int USERNAME_MAX_LENGTH{24};

/** Password min/max length (arbitrarily chosen). */
inline constexpr int PASSWORD_MIN_LENGTH{10};
inline constexpr int PASSWORD_MAX_LENGTH{64};

/** Length of an account recovery key in characters. */
inline constexpr std::size_t RECOVERY_KEY_CHARACTERS{24};

/** Length of an account session token in bytes. */
inline constexpr std::size_t SESSION_TOKEN_BYTES{32};

/** Length of a service ticket in bytes. */
inline constexpr std::size_t SERVICE_TICKET_BYTES{32};

/** The service that a ticket authorizes the client to connect to. */
enum class ServiceTicketAudience : Uint8 {
    WorldServer,
    ChatServer
};

} // End namespace AM
