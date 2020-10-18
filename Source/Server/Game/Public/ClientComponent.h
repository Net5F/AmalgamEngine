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

    /**
     * true if the client has been sent the state of all entities that existed
     * before it connected, else false.
     * TODO: Remove this and add a real area of interest component/system.
     */
    bool isInitialized = false;
};

} // namespace Server
} // namespace AM
