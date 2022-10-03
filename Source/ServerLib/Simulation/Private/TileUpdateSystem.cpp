#include "TileUpdateSystem.h"
#include "World.h"
#include "Network.h"
#include "ClientSimData.h"
#include "AMAssert.h"
#include "Tracy.hpp"

namespace AM
{
namespace Server
{
TileUpdateSystem::TileUpdateSystem(World& inWorld,
                                   EventDispatcher& inNetworkEventDispatcher,
                                   Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, tileUpdateRequestQueue(inNetworkEventDispatcher)
{
}

void TileUpdateSystem::updateTiles()
{
    ZoneScoped;

    // Process any waiting update requests.
    TileUpdateRequest updateRequest;
    while (tileUpdateRequestQueue.pop(updateRequest)) {
        // Update the map.
        // Note: This doesn't check if the client entity is within any certain
        //       range of the tile or anything. We can add that if it's
        //       useful, I just couldn't immediately think of a use case for it.
        world.tileMap.setTileSpriteLayer(
            updateRequest.tileX, updateRequest.tileY, updateRequest.layerIndex,
            updateRequest.numericID);
    }
}

void TileUpdateSystem::sendTileUpdates()
{
    auto clientView = world.registry.view<ClientSimData>();
    std::unordered_map<TilePosition, std::size_t>& dirtyTiles{
        world.tileMap.getDirtyTiles()};

    // For every tile with dirty state, push it into the working update of 
    // each client in range.
    for (const auto& [tilePos, lowestDirtyLayerIndex] : dirtyTiles) {
        // Calc how many layers the tile has, starting at the lowest dirty layer.
        // Note: This tile might be fully cleared (no layers).
        const Tile& tile{world.tileMap.getTile(tilePos.x, tilePos.y)};
        std::size_t layerCount{tile.spriteLayers.size()
                               - lowestDirtyLayerIndex};
        AM_ASSERT(lowestDirtyLayerIndex <= SDL_MAX_UINT8,
                  "Too large for Uint8.");
        AM_ASSERT(layerCount <= SDL_MAX_UINT8, "Too large for Uint8.");

        // Get the list of clients that are in range of this tile.
        // Note: This is hardcoded to match ChunkUpdateSystem.
        ChunkPosition centerChunk{TilePosition{tilePos.x, tilePos.y}};
        ChunkExtent chunkExtent{(centerChunk.x - 1), (centerChunk.y - 1), 3, 3};
        chunkExtent.intersectWith(world.tileMap.getChunkExtent());

        // Add this tile to the working update of all clients that are in range.
        std::vector<entt::entity>& entitiesInRange{
            world.entityLocator.getEntitiesFine(chunkExtent)};
        for (entt::entity entity : entitiesInRange) {
            ClientSimData& client{clientView.get<ClientSimData>(entity)};

            // Push the tile info.
            workingUpdates[client.netID].tileInfo.emplace_back(
                tilePos.x, tilePos.y, static_cast<Uint8>(layerCount),
                static_cast<Uint8>(lowestDirtyLayerIndex));

            // Push the numericID of the lowest updated layer and all layers 
            // above it.
            for (std::size_t i = 0; i < layerCount; ++i) {
                int numericID{tile.spriteLayers[lowestDirtyLayerIndex + i]
                                  .sprite.numericID};
                workingUpdates[client.netID].updatedLayers.push_back(numericID);
            }
        }
    }

    // Send all the updates.
    for (auto& [netID, tileUpdate] : workingUpdates) {
        LOG_INFO("Sending update with %u tiles, %u layers",
                 tileUpdate.tileInfo.size(), tileUpdate.updatedLayers.size());
        network.serializeAndSend<TileUpdate>(netID, tileUpdate);
    }
    workingUpdates.clear();

    // The dirty tile map state is now clean, clear the tracked dirty tiles.
    dirtyTiles.clear();
}

} // End namespace Server
} // End namespace AM
