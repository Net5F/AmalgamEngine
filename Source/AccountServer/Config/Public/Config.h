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
    // Client network
    //-------------------------------------------------------------------------
    /** The port that the server listens for incoming client connections on. */
    static constexpr unsigned int SERVER_CLIENT_PORT{41498};

    /** The port that the server listens for incoming service connections on. */
    static constexpr unsigned int SERVER_SERVICE_PORT{41497};

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

    /** The maximum amount of outgoing bytes we'll allow at once before
        considering the client to be hostile and closing the connection. */
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{4000};

    /** The maximum number of outgoing writes we'll allow at once before
        considering the client to be hostile and closing the connection. */
    static constexpr std::size_t MAX_QUEUED_WRITES{10};

    /** The max size, in bytes, for an incoming message payload.
        Kept to a reasonable size to minimize data usage per connection. */
    static constexpr std::size_t MAX_READ_PAYLOAD_SIZE{500};

    /** The max size, in bytes, for an outgoing message payload.
        Kept to a reasonable size to minimize data usage per connection. */
    static constexpr std::size_t MAX_WRITE_PAYLOAD_SIZE{500};

    //-------------------------------------------------------------------------
    // Service network
    //-------------------------------------------------------------------------
    /** The maximum number of service connections that we will allow at once.
        Only 1 world server is supported, but a few extra slots let it
        reconnect while a stale connection is still being cleaned up. */
    static constexpr unsigned int MAX_SERVICE_CONNECTIONS{4};

    /** How long a connected service can be idle before timing out.
        Services send periodic heartbeats, so this only triggers if the
        service has gone away. Services are expected to reconnect. */
    static constexpr double SERVICE_IDLE_TIMEOUT_S{60};

    /** The maximum amount of outgoing bytes we'll allow at once on a service
        connection. Larger than the client limit, since a service may send
        bursts of requests on behalf of many users. */
    static constexpr std::size_t SERVICE_MAX_QUEUED_WRITE_BYTES{64000};

    /** The maximum number of outgoing writes we'll allow at once on a service
        connection. */
    static constexpr std::size_t SERVICE_MAX_QUEUED_WRITES{1000};

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
