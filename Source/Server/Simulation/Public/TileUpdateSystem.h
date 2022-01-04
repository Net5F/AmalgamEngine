#pragma once

#include "QueuedEvents.h"
#include "TileUpdateRequest.h"

namespace AM
{
namespace Server
{
class World;
class Network;

/**
 * Processes tile update requests. If the request is valid, updates the
 * map and sends the new map state to all nearby clients.
 */
class TileUpdateSystem
{
public:
    TileUpdateSystem(World& inWorld, EventDispatcher& inNetworkEventDispatcher,
                     Network& inNetwork);

    /**
     * Processes tile updates, updating the map and sending the new map state
     * to all relevant clients.
     */
    void updateTiles();

private:
    /** Used to access the player entity and components. */
    World& world;
    /** Used to send chunk update request messages. */
    Network& network;

    EventQueue<TileUpdateRequest> tileUpdateRequestQueue;
};

} // namespace Server
} // namespace AM
