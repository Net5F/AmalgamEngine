#pragma once

#include <SDL3/SDL_stdinc.h>

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

} // End namespace AM
