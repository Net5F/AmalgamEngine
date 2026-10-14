#pragma once

#include "PlayerPositionsUpdate.h"
#include "PlayersLeftWorld.h"
#include <SDL3/SDL_stdinc.h>
#include <algorithm>
#include <cstddef>

namespace AM
{
namespace ChatServer
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
    static constexpr unsigned int SERVER_CLIENT_PORT{41496};

    /** The port that the server listens for incoming service connections on. */
    static constexpr unsigned int SERVER_SERVICE_PORT{41495};

    /** The maximum number of clients that we will allow at once. */
    static constexpr unsigned int MAX_CLIENTS{1000};

    /** How long a connecting (or handshaking) client can be idle before timing
        out. */
    static constexpr double CONNECTING_TIMEOUT_S{5};

    /** How long we'll wait after receiving the first byte of a message before
        timing out. Prevents slowloris-style attacks. */
    static constexpr double PARTIAL_RECEIVE_TIMEOUT_S{10};

    /** How long a connected client can be idle before timing out.
        Clients send periodic heartbeats, so this only triggers if the client
        has gone away. */
    static constexpr double IDLE_TIMEOUT_S{30};

    /** The maximum amount of outgoing bytes we'll allow at once before
        considering the client to be hostile and closing the connection.
        Larger than the AccountServer's, since a busy channel may send a burst
        of messages to every client. */
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{32000};

    /** The maximum number of outgoing writes we'll allow at once before
        considering the client to be hostile and closing the connection. */
    static constexpr std::size_t MAX_QUEUED_WRITES{100};

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
        connection. */
    static constexpr std::size_t SERVICE_MAX_QUEUED_WRITE_BYTES{64000};

    /** The maximum number of outgoing writes we'll allow at once on a service
        connection. */
    static constexpr std::size_t SERVICE_MAX_QUEUED_WRITES{1000};

    /** The max size, in bytes, for an incoming service message payload.
        Must fit the largest message that the world server sends. */
    static constexpr std::size_t SERVICE_MAX_READ_PAYLOAD_SIZE{
        std::max(PlayerPositionsUpdate::MAX_SERIALIZED_SIZE,
                 PlayersLeftWorld::MAX_SERIALIZED_SIZE)};

    /** The max size, in bytes, for an outgoing service message payload. */
    static constexpr std::size_t SERVICE_MAX_WRITE_PAYLOAD_SIZE{500};
};

} // End namespace ChatServer
} // End namespace AM
