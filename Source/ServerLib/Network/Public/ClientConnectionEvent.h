#pragma once

#include "NetworkID.h"
#include <variant>

namespace AM
{
namespace Server
{
/**
 * Used to tell the simulation that a client was connected.
 */
struct ClientConnected {
    /** The ID of the client that connected. */
    NetworkID clientID{0};
};

/**
 * Used to tell the simulation that a client was disconnected.
 */
struct ClientDisconnected {
    /** The ID of the client that disconnected. */
    NetworkID clientID{0};
};

/** Used to synchronize connect/disconnect events. Without this, we may observe 
    a disconnect for a client before processing the connect event. */
using ClientConnectionEvent
    = std::variant<ClientConnected, ClientDisconnected>;

} // End namespace Server
} // End namespace AM
