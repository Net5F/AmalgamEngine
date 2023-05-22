#pragma once

#include "QueuedEvents.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"

namespace AM
{
namespace Client
{
class World;
class Network;

/**
 * Processes tile updates.
 *
 * See updateTiles() comment for more info.
 *
 * Note: If an updated tile collides with an entity, visual desyncs can occur.
 *       This client-side visual desync can occur for NPC entities as well as
 *       the player entity.
 *       This desync is easily fixed, the offending entity just needs to change
 *       inputs. The received authoritative state will correct the desync.
 *       In the future, we may wish to build a more sophisticated solution.
 */
class TileUpdateSystem
{
public:
    TileUpdateSystem(World& inWorld, EventDispatcher& inUiEventDispatcher,
                     Network& inNetwork);

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

    /**
     * Adds the tile layer to the map.
     */
    void addTileLayer(const TileAddLayer& addLayer);

    /**
     * Removes the tile layer from the map.
     */
    void remTileLayer(const TileRemoveLayer& remLayer);

    /**
     * Clears the tile layers from the map.
     */
    void clearTileLayers(const TileClearLayers& clearLayers);

    /**
     * Clears the tile layers from the map.
     */
    void clearExtentLayers(
        const TileExtentClearLayers& clearExtentLayers);

    /** Used to access the tile map. */
    World& world;
    /** Used to send tile update request messages. */
    Network& network;

    /** Tile update requests, received from the UI. */
    EventQueue<TileAddLayer> addLayerRequestQueue;
    EventQueue<TileRemoveLayer> removeLayerRequestQueue;
    EventQueue<TileClearLayers> clearLayersRequestQueue;
    EventQueue<TileExtentClearLayers> extentClearLayersRequestQueue;

    /** Tile updates, received from the network. */
    EventQueue<TileAddLayer> addLayerQueue;
    EventQueue<TileRemoveLayer> removeLayerQueue;
    EventQueue<TileClearLayers> clearLayersQueue;
    EventQueue<TileExtentClearLayers> extentClearLayersQueue;
};

} // namespace Client
} // namespace AM
