#pragma once

#include "QueuedEvents.h"
#include "TileUpdateRequest.h"
#include "TileUpdate.h"
#include <unordered_map>

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
     * Processes tile updates and updates the world's tile map.
     */
    void updateTiles();

    /**
     * Sends any dirty tile state to all nearby clients.
     */
    void sendTileUpdates();

private:
    /** Used to access the entity registry, locator, and the tile map. */
    World& world;
    /** Used to send chunk update request messages. */
    Network& network;

    /** Holds tile updates as we iterate the dirty tiles and figure out what 
        needs to be sent to each client. */
    std::unordered_map<NetworkID, TileUpdate> workingUpdates;

    EventQueue<TileUpdateRequest> tileUpdateRequestQueue;
};

} // namespace Server
} // namespace AM
