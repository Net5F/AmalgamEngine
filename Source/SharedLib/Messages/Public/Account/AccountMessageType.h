#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * The types of messages that we send across the network from Client <-> Account
 * Server.
 *
 * For message descriptions, see their definitions in Shared/Messages/Public.
 */
enum class AccountMessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet,

    // Client -> Server Messages
    AccountRegisterRequest,

    // Server -> Client Messages
    AccountRegisterResponse,

    // Bidirectional Messages
};

} // End namespace AM
