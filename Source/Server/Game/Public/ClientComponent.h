#pragma once

#include "NetworkDefs.h"

namespace AM
{
namespace Server
{
/**
 * Indicates that an entity is associated with a Client.
 * Tracks that Client's NetworkID so the Game knows which one to send
 * messages to.
 *
 * Also used to remove the entity from the game when the Client disconnects.
 */
struct ClientComponent {
public:
    /** The network ID associated with this client. */
    NetworkID netID = 0;

    /** Tracks if a message from this client was received late and had to be
        dropped. */
    bool messageWasDropped = false;
};

} // namespace Server
} // namespace AM
