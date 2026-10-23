#pragma once

#include "ConnectionHandle.h"
#include "DisconnectNotice.h"
#include <optional>
#include <string>
#include <variant>

namespace AM
{
namespace WorldServer
{
/**
 * Used to tell the simulation that a client was connected and authenticated.
 */
struct ClientConnected {
    /** The ID of the client that connected. */
    ConnectionHandle clientHandle{};

    /** The ID of the account that the client authenticated as. */
    Sint64 accountID{0};

    /** The username of the account that the client authenticated as. */
    std::string username{};
};

/**
 * Used to tell the simulation that a client was disconnected.
 *
 * Note: This is only sent for clients that a ClientConnected was sent for.
 */
struct ClientDisconnected {
    /** The ID of the client that disconnected. */
    ConnectionHandle clientHandle{};

    /** If we initiated the disconnect (see WorldClientEndpoint::disconnect()),
        this is the reason that was given. If empty, the connection was lost
        (e.g. the client disconnected or timed out). */
    std::optional<DisconnectNotice::Reason> reason{};
};

/** Used to synchronize connect/disconnect events. Without this, we may observe
    a disconnect for a client before processing the connect event. */
using ClientConnectionEvent = std::variant<ClientConnected, ClientDisconnected>;

} // End namespace WorldServer
} // End namespace AM
