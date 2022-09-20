#include "TileUpdateSystem.h"
#include "World.h"
#include "Network.h"
#include "ClientSimData.h"
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
    std::unordered_set<TilePosition>& dirtyTiles{world.tileMap.getDirtyTiles()};

    // For every tile with dirty state, push it into the working update of 
    // each client in range.
    for (const TilePosition& position : dirtyTiles) {
        const Tile& tile{world.tileMap.getTile(position.x, position.y)};

        // Get the list of clients that are in range of this tile.
        // Note: This is hardcoded to match ChunkUpdateSystem.
        ChunkPosition centerChunk{TilePosition{position.x, position.y}};
        ChunkExtent chunkExtent{(centerChunk.x - 1), (centerChunk.y - 1), 3, 3};
        chunkExtent.intersectWith(world.tileMap.getChunkExtent());

        // Add this tile to the working update of all clients that are in range.
        std::vector<entt::entity>& entitiesInRange{
            world.entityLocator.getEntitiesFine(chunkExtent)};
        for (entt::entity entity : entitiesInRange) {
            ClientSimData& client{clientView.get<ClientSimData>(entity)};

            // Push the tile info.
            workingUpdates[client.netID].tileInfo.emplace_back(
                position.x, position.y,
                static_cast<unsigned int>(tile.spriteLayers.size()));

            // Push the numericID of each of the tile's layers.
            for (const Tile::SpriteLayer& layer : tile.spriteLayers) {
                workingUpdates[client.netID].updatedLayers.push_back(
                    layer.sprite.numericID);
            }
        }
    }

    // Send all the updates.
    for (auto& [netID, tileUpdate] : workingUpdates) {
        network.serializeAndSend<TileUpdate>(netID, tileUpdate);
    }
    workingUpdates.clear();

    // The dirty tile map state is now clean, clear the set.
    dirtyTiles.clear();
}

} // End namespace Server
} // End namespace AM
