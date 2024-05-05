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
    if (addLayerRequest.layerType == TileLayer::Type::Floor) {
        world.tileMap.setFloor(addLayerRequest.tileX, addLayerRequest.tileY,
                               addLayerRequest.graphicSetID);
    }
    else if (addLayerRequest.layerType == TileLayer::Type::FloorCovering) {
        world.tileMap.addFloorCovering(
            addLayerRequest.tileX, addLayerRequest.tileY,
            addLayerRequest.graphicSetID,
            static_cast<Rotation::Direction>(addLayerRequest.graphicIndex));
    }
    else if (addLayerRequest.layerType == TileLayer::Type::Wall) {
        world.tileMap.addWall(
            addLayerRequest.tileX, addLayerRequest.tileY,
            addLayerRequest.graphicSetID,
            static_cast<Wall::Type>(addLayerRequest.graphicIndex));
    }
    else if (addLayerRequest.layerType == TileLayer::Type::Object) {
        world.tileMap.addObject(
            addLayerRequest.tileX, addLayerRequest.tileY,
            addLayerRequest.graphicSetID,
            static_cast<Rotation::Direction>(addLayerRequest.graphicIndex));
    }
}

void TileUpdateSystem::remTileLayer(const TileRemoveLayer& remLayerRequest)
{
    if (remLayerRequest.layerType == TileLayer::Type::Floor) {
        // Note: We normally won't receive this, since the server uses the
        //       clearTileLayers path for removing floors.
        world.tileMap.remFloor(remLayerRequest.tileX, remLayerRequest.tileY);
    }
    else if (remLayerRequest.layerType == TileLayer::Type::FloorCovering) {
        world.tileMap.remFloorCovering(
            remLayerRequest.tileX, remLayerRequest.tileY,
            remLayerRequest.graphicSetID,
            static_cast<Rotation::Direction>(remLayerRequest.graphicIndex));
    }
    else if (remLayerRequest.layerType == TileLayer::Type::Wall) {
        world.tileMap.remWall(
            remLayerRequest.tileX, remLayerRequest.tileY,
            static_cast<Wall::Type>(remLayerRequest.graphicIndex));
    }
    else if (remLayerRequest.layerType == TileLayer::Type::Object) {
        world.tileMap.remObject(
            remLayerRequest.tileX, remLayerRequest.tileY,
            remLayerRequest.graphicSetID,
            static_cast<Rotation::Direction>(remLayerRequest.graphicIndex));
    }
}

void TileUpdateSystem::clearTileLayers(
    const TileClearLayers& clearLayersRequest)
{
    world.tileMap.clearTileLayers(clearLayersRequest.tileX,
                                  clearLayersRequest.tileY,
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
