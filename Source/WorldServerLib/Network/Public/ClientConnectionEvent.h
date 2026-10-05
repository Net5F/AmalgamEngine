#pragma once

#include "NetworkID.h"
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
    NetworkID clientID{0};

    /** The ID of the account that the client authenticated as. */
    Sint64 accountID{0};
};

/**
 * Used to tell the simulation that a client was disconnected.
 *
 * Note: This is only sent for clients that a ClientConnected was sent for.
 */
struct ClientDisconnected {
    /** The ID of the client that disconnected. */
    NetworkID clientID{0};
};

/** Used to synchronize connect/disconnect events. Without this, we may observe
    a disconnect for a client before processing the connect event. */
using ClientConnectionEvent = std::variant<ClientConnected, ClientDisconnected>;

} // End namespace WorldServer
} // End namespace AM
