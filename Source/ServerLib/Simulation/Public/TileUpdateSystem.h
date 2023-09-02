#pragma once

#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Server
{
class World;
class Network;
class ISimulationExtension;

/**
 * Processes tile update requests sent by clients. If a request is valid,
 * updates the map.
 * Also, detects changes to the tile map and sends the new map state to all
 * nearby clients.
 */
class TileUpdateSystem
{
public:
    TileUpdateSystem(World& inWorld, Network& inNetwork);

    /**
     * Processes tile updates and updates the world's tile map.
     */
    void updateTiles();

    /**
     * Sends any dirty tile state to all nearby clients.
     */
    void sendTileUpdates();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * If the given request is valid, adds the tile layer to the map.
     */
    void addTileLayer(const TileAddLayer& addLayerRequest);

    /**
     * If the given request is valid, removes the tile layer from the map.
     */
    void remTileLayer(const TileRemoveLayer& remLayerRequest);

    /**
     * If the given request is valid, clears the tile layers from the map.
     */
    void clearTileLayers(const TileClearLayers& clearLayersRequest);

    /**
     * If the given request is valid, clears the tile layers from the map.
     */
    void clearExtentLayers(
        const TileExtentClearLayers& clearExtentLayersRequest);

    /** Used to access the entity registry, locator, and the tile map. */
    World& world;
    /** Used to send tile update requests and receive tile updates. */
    Network& network;

    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if tile updates are valid. */
    ISimulationExtension* extension;

    EventQueue<TileAddLayer> addLayerRequestQueue;
    EventQueue<TileRemoveLayer> removeLayerRequestQueue;
    EventQueue<TileClearLayers> clearLayersRequestQueue;
    EventQueue<TileExtentClearLayers> extentClearLayersRequestQueue;
};

} // namespace Server
} // namespace AM
