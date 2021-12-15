#pragma once

#include "NetworkDefs.h"

namespace AM
{
namespace Server
{
/**
 * Holds any Client data that is relevant to the sim systems.
 *
 * Tracks the Client's NetworkID so the sim knows where to send messages
 * related to the entity. Also uses the netID to remove the entity from the sim
 * when the Client disconnects.
 *
 * Additionally, manages any other client-specific data like AoI bounds.
 */
struct ClientSimData {
public:
    /** The network ID associated with this client. */
    NetworkID netID{0};

    /** Tracks if an input from this client was received late and had to be
        dropped. */
    bool inputWasDropped{false};
};

} // namespace Server
} // namespace AM
