#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Messages sent between the account server and our other internal services
 * (chat server, world server).
 */
enum class AccountServiceMessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet,

    // Service -> Account Server
    ConsumeServiceTicketRequest,
    ServiceHeartbeat,

    // Account Server -> Service
    ConsumeServiceTicketResponse,
    ServiceHeartbeatResponse,
};

} // End namespace AM
