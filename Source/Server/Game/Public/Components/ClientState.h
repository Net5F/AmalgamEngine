#pragma once

#include "NetworkDefs.h"

namespace AM
{
namespace Server
{
/**
 * Associates the attached entity with a Client.
 * Tracks the Client's NetworkID so the sim knows where to send messages
 * related to the entity.
 *
 * Also used to remove the entity from the sim when the Client disconnects.
 */
struct ClientState {
public:
    /** The network ID associated with this client. */
    NetworkID netID{0};

    /** Tracks if a message from this client was received late and had to be
        dropped. */
    bool messageWasDropped{false};
};

} // namespace Server
} // namespace AM
