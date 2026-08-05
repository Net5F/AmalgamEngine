#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{
namespace AccountServer
{
/**
 * Module-specific configuration data.
 */
class Config
{
public:
    //-------------------------------------------------------------------------
    // Network
    //-------------------------------------------------------------------------
    /** The port that the server listens for incoming client connections on. */
    static constexpr unsigned int SERVER_PORT{41498};

    /** The maximum number of clients that we will allow. */
    static constexpr unsigned int MAX_CLIENTS{1000};

    /** How long we should wait before considering the client to be timed out.
        Arbitrarily chosen. If too high, we set ourselves up to take a huge
        spike of data for a very late client. */
    static constexpr double CLIENT_TIMEOUT_S{4};

    //-------------------------------------------------------------------------
    // Account sessions
    //-------------------------------------------------------------------------
    /** How long an account session can remain idle before expiring. */
    static constexpr Sint64 ACCOUNT_SESSION_IDLE_TIMEOUT_S{8 * 60 * 60};

    /** Maximum lifetime of an account session, regardless of activity. */
    static constexpr Sint64 ACCOUNT_SESSION_ABSOLUTE_TIMEOUT_S{24 * 60 * 60};

    /** How long a service ticket remains valid. */
    static constexpr Sint64 SERVICE_TICKET_LIFETIME_S{60};
};

} // End namespace AccountServer
} // End namespace AM
