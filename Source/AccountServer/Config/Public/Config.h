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

    /** The maximum number of clients that we will allow at once. */
    static constexpr unsigned int MAX_CLIENTS{1000};

    /** How long a connecting (or handshaking) client can be idle before timing
        out. */
    static constexpr double CONNECTING_TIMEOUT_S{5};

    /** How long we'll wait after receiving the first byte of a message before 
        timing out. Prevents slowloris-style attacks. */
    static constexpr double PARTIAL_RECEIVE_TIMEOUT_S{10};

    /** How long a connected client can be idle before timing out. */
    static constexpr double IDLE_TIMEOUT_S{10};

    /** How long a connected client can not consume our written data before 
        timing out. */
    static constexpr double WRITE_TIMEOUT_S{10};

    /** The maximum amount of outgoing bytes we'll allow at once before 
        considering the client to be hostile and closing the connection. */
    static constexpr double MAX_QUEUED_WRITE_BYTES{10};

    /** The maximum number of outgoing writes we'll allow at once before 
        considering the client to be hostile and closing the connection. */
    static constexpr double MAX_QUEUED_WRITES{10};

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
