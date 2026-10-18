#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{
namespace Client
{

/**
 * Communicates a change in the ChatServer connection lifecycle.
 *
 * Once the connection fails, is rejected, or disconnects, it has been fully
 * torn down and a new connection (with a new ticket) is required.
 */
struct ChatConnectionEvent {
    enum class Type {
        /** ChatServer accepted our ticket. Chat can now be sent and
            received. */
        Authenticated,
        /** ChatServer rejected our ticket. */
        InvalidTicket,
        /** ChatServer failed to validate our ticket. */
        Rejected,
        /** We failed to connect to ChatServer. */
        ConnectionFailed,
        /** We lost our connection to ChatServer. */
        Disconnected
    };

    Type type{Type::Disconnected};

    /** The ID returned by the ChatClientEndpoint::connect() call that started
        this connection. Used to discard events from old connections. */
    Uint32 connectionID{0};
};

} // namespace Client
} // namespace AM
