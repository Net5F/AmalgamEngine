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
    TileUpdateSystem(World& inWorld, Network& inNetwork);

    /**
     * Processes received tile updates, applying them to the tile map.
     */
    void updateTiles();

private:
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
    /** Used to receive tile updates. */
    Network& network;

    /** Tile updates, received from the network. */
    EventQueue<TileAddLayer> addLayerQueue;
    EventQueue<TileRemoveLayer> removeLayerQueue;
    EventQueue<TileClearLayers> clearLayersQueue;
    EventQueue<TileExtentClearLayers> extentClearLayersQueue;
};

} // namespace Client
} // namespace AM
