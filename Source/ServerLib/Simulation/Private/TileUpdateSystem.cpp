#include "TileUpdateSystem.h"
#include "World.h"
#include "Network.h"
#include "ISimulationExtension.h"
#include "ClientSimData.h"
#include "ChunkPosition.h"
#include "TilePosition.h"
#include "AMAssert.h"
#include "tracy/Tracy.hpp"
#include <variant>

namespace AM
{
namespace Server
{
/** Returns the extent that's in range of a given tile update.
    Note: This matches ChunkUpdateSystem's behavior, which includes all
          directly surrounding chunks. */
struct InRangeExtentGetter {
    TileMap& tileMap;

    ChunkExtent operator()(const TileExtentClearLayers& tileUpdate)
    {
        ChunkExtent chunkExtent{tileUpdate.tileExtent};
        chunkExtent.x -= 1;
        chunkExtent.y -= 1;
        chunkExtent.xLength += 2;
        chunkExtent.yLength += 2;
        chunkExtent.intersectWith(tileMap.getChunkExtent());
        return chunkExtent;
    }

    // TileAddLayer, TileRemoveLayer, TileClearLayers
    template<typename T>
    ChunkExtent operator()(const T& tileUpdate)
    {
        ChunkPosition centerChunk{
            TilePosition{tileUpdate.tileX, tileUpdate.tileY}};
        ChunkExtent chunkExtent{(centerChunk.x - 1), (centerChunk.y - 1), 3, 3};
        chunkExtent.intersectWith(tileMap.getChunkExtent());
        return chunkExtent;
    }
};

/** Sends the given tile update to the currently set client netID. */
struct UpdateSender {
    Network& network;
    NetworkID netID{};

    template<typename T>
    void operator()(const T& tileUpdate)
    {
        network.serializeAndSend<T>(netID, tileUpdate);
    }
};

TileUpdateSystem::TileUpdateSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, extension{nullptr}
, addLayerRequestQueue{network.getEventDispatcher()}
, removeLayerRequestQueue{network.getEventDispatcher()}
, clearLayersRequestQueue{network.getEventDispatcher()}
, extentClearLayersRequestQueue{network.getEventDispatcher()}
{
}

void TileUpdateSystem::updateTiles()
{
    ZoneScoped;

    // Process any waiting update requests.
    TileAddLayer addLayerRequest{};
    while (addLayerRequestQueue.pop(addLayerRequest)) {
        addTileLayer(addLayerRequest);
    }

    TileRemoveLayer removeLayerRequest{};
    while (removeLayerRequestQueue.pop(removeLayerRequest)) {
        remTileLayer(removeLayerRequest);
    }

    TileClearLayers clearLayersRequest{};
    while (clearLayersRequestQueue.pop(clearLayersRequest)) {
        clearTileLayers(clearLayersRequest);
    }

    TileExtentClearLayers clearExtentLayersRequest{};
    while (extentClearLayersRequestQueue.pop(clearExtentLayersRequest)) {
        clearExtentLayers(clearExtentLayersRequest);
    }
}

void TileUpdateSystem::sendTileUpdates()
{
    auto clientView = world.registry.view<ClientSimData>();
    const std::vector<TileMapBase::TileUpdateVariant>& tileUpdateHistory{
        world.tileMap.getTileUpdateHistory()};

    // For every tile update that occurred since we last sent updates.
    InRangeExtentGetter extentGetter{world.tileMap};
    UpdateSender updateSender{network};
    for (const auto& updateVariant : tileUpdateHistory) {
        // Find the entities that are in range of this update.
        ChunkExtent inRangeExtent{std::visit(extentGetter, updateVariant)};
        std::vector<entt::entity>& entitiesInRange{
            world.entityLocator.getEntities(inRangeExtent)};

        // Send the update to all of the in-range clients.
        for (entt::entity entity : entitiesInRange) {
            if (world.registry.all_of<ClientSimData>(entity)) {
                ClientSimData& client{clientView.get<ClientSimData>(entity)};
                updateSender.netID = client.netID;
                std::visit(updateSender, updateVariant);
            }
        }
    }

    world.tileMap.clearTileUpdateHistory();
}

void TileUpdateSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = std::move(inExtension);
}

void TileUpdateSystem::addTileLayer(const TileAddLayer& addLayerRequest)
{
    // If the project says the tile isn't editable, skip this request.
    if ((extension != nullptr)
        && !(extension->isTileExtentEditable(
            addLayerRequest.netID,
            {addLayerRequest.tileX, addLayerRequest.tileY, 1, 1}))) {
        return;
    }

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
    // If the project says the tile isn't editable, skip this request.
    if ((extension != nullptr)
        && !(extension->isTileExtentEditable(
            remLayerRequest.netID,
            {remLayerRequest.tileX, remLayerRequest.tileY, 1, 1}))) {
        return;
    }

    if (remLayerRequest.layerType == TileLayer::Type::Floor) {
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
    // If the project says the tile isn't editable, skip this request.
    if ((extension != nullptr)
        && !(extension->isTileExtentEditable(
            clearLayersRequest.netID,
            {clearLayersRequest.tileX, clearLayersRequest.tileY, 1, 1}))) {
        return;
    }

    world.tileMap.clearTileLayers(clearLayersRequest.tileX,
                                  clearLayersRequest.tileY,
                                  clearLayersRequest.layerTypesToClear);
}

void TileUpdateSystem::clearExtentLayers(
    const TileExtentClearLayers& clearExtentLayersRequest)
{
    // If the project says the extent isn't editable, skip this request.
    if ((extension != nullptr)
        && !(extension->isTileExtentEditable(
            clearExtentLayersRequest.netID,
            clearExtentLayersRequest.tileExtent))) {
        return;
    }

    world.tileMap.clearExtentLayers(clearExtentLayersRequest.tileExtent,
                                    clearExtentLayersRequest.layerTypesToClear);
}

} // End namespace Server
} // End namespace AM
