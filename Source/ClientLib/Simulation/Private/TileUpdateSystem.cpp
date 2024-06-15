#include "TileUpdateSystem.h"
#include "World.h"
#include "Network.h"
#include "AMAssert.h"
#include <variant>

namespace AM
{
namespace Client
{
TileUpdateSystem::TileUpdateSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, addLayerQueue{network.getEventDispatcher()}
, removeLayerQueue{network.getEventDispatcher()}
, clearLayersQueue{network.getEventDispatcher()}
, extentClearLayersQueue{network.getEventDispatcher()}
{
}

void TileUpdateSystem::updateTiles()
{
    // Disable auto collision rebuild (it's more efficient to do it all after).
    world.tileMap.setAutoRebuildCollision(false);

    // Process any waiting tile updates from the server.
    TileClearLayers tileClearLayers{};
    while (clearLayersQueue.pop(tileClearLayers)) {
        clearTileLayers(tileClearLayers);
    }

    TileExtentClearLayers tileExtentClearLayers{};
    while (extentClearLayersQueue.pop(tileExtentClearLayers)) {
        clearExtentLayers(tileExtentClearLayers);
    }

    TileAddLayer tileAddLayer{};
    while (addLayerQueue.pop(tileAddLayer)) {
        addTileLayer(tileAddLayer);
    }

    TileRemoveLayer tileRemoveLayer{};
    while (removeLayerQueue.pop(tileRemoveLayer)) {
        remTileLayer(tileRemoveLayer);
    }

    // Re-enable auto collision rebuild (rebuilds any dirty tiles).
    world.tileMap.setAutoRebuildCollision(true);
}

void TileUpdateSystem::addTileLayer(const TileAddLayer& addLayerRequest)
{
    if (addLayerRequest.layerType == TileLayer::Type::Terrain) {
        world.tileMap.addTerrain(
            addLayerRequest.tilePosition, addLayerRequest.graphicSetID,
            static_cast<Terrain::Height>(addLayerRequest.graphicValue));
    }
    else if (addLayerRequest.layerType == TileLayer::Type::Floor) {
        world.tileMap.addFloor(
            addLayerRequest.tilePosition, addLayerRequest.graphicSetID,
            static_cast<Rotation::Direction>(addLayerRequest.graphicValue));
    }
    else if (addLayerRequest.layerType == TileLayer::Type::Wall) {
        world.tileMap.addWall(
            addLayerRequest.tilePosition, addLayerRequest.graphicSetID,
            static_cast<Wall::Type>(addLayerRequest.graphicValue));
    }
    else if (addLayerRequest.layerType == TileLayer::Type::Object) {
        world.tileMap.addObject(
            addLayerRequest.tilePosition, addLayerRequest.graphicSetID,
            static_cast<Rotation::Direction>(addLayerRequest.graphicValue));
    }
}

void TileUpdateSystem::remTileLayer(const TileRemoveLayer& remLayerRequest)
{
    if (remLayerRequest.layerType == TileLayer::Type::Terrain) {
        world.tileMap.remTerrain(
            remLayerRequest.tilePosition, remLayerRequest.graphicSetID,
            static_cast<Terrain::Height>(remLayerRequest.graphicValue));
    }
    else if (remLayerRequest.layerType == TileLayer::Type::Floor) {
        world.tileMap.remFloor(
            remLayerRequest.tilePosition, remLayerRequest.graphicSetID,
            static_cast<Rotation::Direction>(remLayerRequest.graphicValue));
    }
    else if (remLayerRequest.layerType == TileLayer::Type::Wall) {
        world.tileMap.remWall(
            remLayerRequest.tilePosition,
            static_cast<Wall::Type>(remLayerRequest.graphicValue));
    }
    else if (remLayerRequest.layerType == TileLayer::Type::Object) {
        world.tileMap.remObject(
            remLayerRequest.tilePosition, remLayerRequest.graphicSetID,
            static_cast<Rotation::Direction>(remLayerRequest.graphicValue));
    }
}

void TileUpdateSystem::clearTileLayers(
    const TileClearLayers& clearLayersRequest)
{
    world.tileMap.clearTileLayers(clearLayersRequest.tilePosition,
                                  clearLayersRequest.layerTypesToClear);
}

void TileUpdateSystem::clearExtentLayers(
    const TileExtentClearLayers& clearExtentLayersRequest)
{
    world.tileMap.clearExtentLayers(clearExtentLayersRequest.tileExtent,
                                    clearExtentLayersRequest.layerTypesToClear);
}

} // End namespace Client
} // End namespace AM
