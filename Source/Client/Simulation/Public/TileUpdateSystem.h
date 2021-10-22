#pragma once

#include "QueuedEvents.h"
#include "TileUpdate.h"
#include "TileUpdateRequest.h"
#include <SDL2/SDL_stdinc.h>

namespace AM
{
struct ChunkPosition;

namespace Client
{
class Simulation;
class World;
class Network;
class SpriteData;
class TileMap;

/**
 * Processes tile updates.
 *
 * On the client -> server side, processes tile update requests received
 * from the UI, updates the tile map, and forwards the requests to the
 * server.
 *
 * On the server -> client side, applies received tile updates to the
 * tile map.
 */
class TileUpdateSystem
{
public:
    // TODO: Do we need the sim?
    TileUpdateSystem(Simulation& inSim, World& inWorld
                     , EventDispatcher& inUiEventDispatcher
                     , EventDispatcher& inNetworkEventDispatcher
                     , Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Processes tile updates (see class comment).
     */
    void updateTiles();

private:
    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access the player entity and components. */
    World& world;
    /** Used to send chunk update request messages. */
    Network& network;
    /** Used to access sprites while updating tiles. */
    SpriteData& spriteData;

    /** Tile update requests, received from the UI. */
    EventQueue<TileUpdateRequest> tileUpdateRequestQueue;
    /** Tile updates, received from the network. */
    EventQueue<TileUpdate> tileUpdateQueue;
};

} // namespace Client
} // namespace AM
