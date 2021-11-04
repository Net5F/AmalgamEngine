#pragma once

#include "QueuedEvents.h"
#include "TileUpdateRequest.h"

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;

/**
 * Processes tile updates.
 *
 * See updateTiles() comment for more info.
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
