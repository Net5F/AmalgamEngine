#pragma once

#include "QueuedEvents.h"
#include "TileUpdate.h"
#include "TileUpdateRequest.h"

namespace AM
{
struct ChunkPosition;

namespace Client
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
    TileUpdateSystem(World& inWorld
                     , EventDispatcher& inUiEventDispatcher
                     , EventDispatcher& inNetworkEventDispatcher
                     , Network& inNetwork);

    /**
     * Processes tile updates.
     *
     * On the client -> server side, processes tile update requests received
     * from the UI, updates the tile map, and sends the requests to the
     * server.
     *
     * On the server -> client side, applies received tile updates to the
     * tile map.
     */
    void updateTiles();

private:
    /**
     * Processes tile update requests from the UI.
     */
    void processUiRequests();

    /**
     * Processes tile updates from the server.
     */
    void processNetworkUpdates();

    /** Used to access the player entity and components. */
    World& world;
    /** Used to send chunk update request messages. */
    Network& network;

    /** Tile update requests, received from the UI. */
    EventQueue<TileUpdateRequest> tileUpdateRequestQueue;
    /** Tile updates, received from the network. */
    EventQueue<TileUpdate> tileUpdateQueue;
};

} // namespace Client
} // namespace AM
