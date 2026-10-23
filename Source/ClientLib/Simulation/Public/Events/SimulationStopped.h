#pragma once

#include "entt/fwd.hpp"

namespace AM
{
namespace Client
{

/**
 * An event to show that our server session has ended (we disconnected, or a
 * connection attempt was aborted).
 *
 * Sent after the connection is closed, so nothing else from the session will
 * be received. Anything that queues messages from the server should discard
 * them when this is received, so they aren't processed in the next session.
 */
struct SimulationStopped {
};

} // namespace Client
} // namespace AM
