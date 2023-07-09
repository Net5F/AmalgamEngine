#include "TileUpdateSystem.h"
#include "World.h"
#include "Network.h"
#include "AMAssert.h"
#include <variant>

namespace AM
{
namespace Client
{
TileUpdateSystem::TileUpdateSystem(World& inWorld,
                                   EventDispatcher& inUiEventDispatcher,
                                   Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, addLayerRequestQueue{inUiEventDispatcher}
, removeLayerRequestQueue{inUiEventDispatcher}
, clearLayersRequestQueue{inUiEventDispatcher}
, extentClearLayersRequestQueue{inUiEventDispatcher}
, addLayerQueue{network.getEventDispatcher()}
, removeLayerQueue{network.getEventDispatcher()}
, clearLayersQueue{network.getEventDispatcher()}
, extentClearLayersQueue{network.getEventDispatcher()}
{
}

void TileUpdateSystem::updateTiles()
{
    // Process tile update requests from the UI.
    processUiRequests();

    // Process tile updates from the server.
    processNetworkUpdates();
}

void TileUpdateSystem::processUiRequests()
{
    // Process any waiting update requests from the UI.
    TileAddLayer addLayerRequest{};
    while (addLayerRequestQueue.pop(addLayerRequest)) {
        network.serializeAndSend<TileAddLayer>(addLayerRequest);
    }

    TileRemoveLayer removeLayerRequest{};
    while (removeLayerRequestQueue.pop(removeLayerRequest)) {
        network.serializeAndSend<TileRemoveLayer>(removeLayerRequest);
    }

    TileClearLayers clearLayersRequest{};
    while (clearLayersRequestQueue.pop(clearLayersRequest)) {
        network.serializeAndSend<TileClearLayers>(clearLayersRequest);
    }

    TileExtentClearLayers clearExtentLayersRequest{};
    while (extentClearLayersRequestQueue.pop(clearExtentLayersRequest)) {
        network.serializeAndSend<TileExtentClearLayers>(
            clearExtentLayersRequest);
    }
}

void TileUpdateSystem::processNetworkUpdates()
{
    // Process any waiting tile updates from the server.
    TileAddLayer tileAddLayer{};
    while (addLayerQueue.pop(tileAddLayer)) {
        addTileLayer(tileAddLayer);
    }

    TileRemoveLayer tileRemoveLayer{};
    while (removeLayerQueue.pop(tileRemoveLayer)) {
        remTileLayer(tileRemoveLayer);
    }

    TileClearLayers tileClearLayers{};
    while (clearLayersQueue.pop(tileClearLayers)) {
        clearTileLayers(tileClearLayers);
    }

    TileExtentClearLayers tileExtentClearLayers{};
    while (extentClearLayersQueue.pop(tileExtentClearLayers)) {
        clearExtentLayers(tileExtentClearLayers);
    }
}

void TileUpdateSystem::addTileLayer(const TileAddLayer& addLayerRequest)
{
    if (addLayerRequest.layerType == TileLayer::Type::Floor) {
        world.tileMap.setFloor(addLayerRequest.tileX, addLayerRequest.tileY,
                               addLayerRequest.spriteSetID);
    }
    else if (addLayerRequest.layerType == TileLayer::Type::FloorCovering) {
        world.tileMap.addFloorCovering(
            addLayerRequest.tileX, addLayerRequest.tileY,
            addLayerRequest.spriteSetID,
            static_cast<Rotation::Direction>(addLayerRequest.spriteIndex));
    }
    else if (addLayerRequest.layerType == TileLayer::Type::Wall) {
        world.tileMap.addWall(
            addLayerRequest.tileX, addLayerRequest.tileY,
            addLayerRequest.spriteSetID,
            static_cast<Wall::Type>(addLayerRequest.spriteIndex));
    }
    else if (addLayerRequest.layerType == TileLayer::Type::Object) {
        world.tileMap.addObject(
            addLayerRequest.tileX, addLayerRequest.tileY,
            addLayerRequest.spriteSetID,
            static_cast<Rotation::Direction>(addLayerRequest.spriteIndex));
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
            remLayerRequest.spriteSetID,
            static_cast<Rotation::Direction>(remLayerRequest.spriteIndex));
    }
    else if (remLayerRequest.layerType == TileLayer::Type::Wall) {
        world.tileMap.remWall(
            remLayerRequest.tileX, remLayerRequest.tileY,
            static_cast<Wall::Type>(remLayerRequest.spriteIndex));
    }
    else if (remLayerRequest.layerType == TileLayer::Type::Object) {
        world.tileMap.remObject(
            remLayerRequest.tileX, remLayerRequest.tileY,
            remLayerRequest.spriteSetID,
            static_cast<Rotation::Direction>(remLayerRequest.spriteIndex));
    }
}

void TileUpdateSystem::clearTileLayers(const TileClearLayers& clearLayersRequest)
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
