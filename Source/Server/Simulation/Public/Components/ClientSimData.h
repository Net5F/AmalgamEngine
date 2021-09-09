#pragma once

#include "NetworkDefs.h"
#include "AreaOfInterest.h"

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

    /** Tracks if a message from this client was received late and had to be
        dropped. */
    bool messageWasDropped{false};

    /** Tracks if this entity needs to be sent its initial chunk data. */
    bool needsInitialChunks{false};

    /** Area of interest bounds. Used to determine if entities are close enough
        to the client entity to be replicated.
        Note: width and height should be set, but the NetworkUpdateSystem
              manages the x/y position. */
    AreaOfInterest aoi{};
};

} // namespace Server
} // namespace AM
