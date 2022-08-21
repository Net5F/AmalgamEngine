#include "TileUpdateSystem.h"
#include "World.h"
#include "Network.h"
#include "ClientSimData.h"
#include "TileUpdate.h"
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

// TODO: Remove timer
Timer timer;
void TileUpdateSystem::sendTileUpdates()
{
    timer.updateSavedTime();
    auto clientView = world.registry.view<ClientSimData>();
    std::unordered_set<TilePosition>& dirtyTiles{world.tileMap.getDirtyTiles()};

    // For every tile with dirty state, send it to all nearby clients.
    unsigned int sentCount{0};
    for (const TilePosition& position : dirtyTiles) {
        const Tile& tile{world.tileMap.getTile(position.x, position.y)};

        // Construct an update message with all of this tile's layers.
        TileUpdate tileUpdate{position.x, position.y, {}};
        for (unsigned int layerIndex = 0; layerIndex < tile.spriteLayers.size();
             ++layerIndex) {
            tileUpdate.numericIDs.push_back(
                tile.spriteLayers[layerIndex].sprite.numericID);
        }

        // Get the list of clients that are in range of this tile.
        // Note: This is hardcoded to match ChunkUpdateSystem.
        ChunkPosition centerChunk{TilePosition{position.x, position.y}};
        ChunkExtent chunkExtent{(centerChunk.x - 1), (centerChunk.y - 1), 3, 3};
        chunkExtent.intersectWith(world.tileMap.getChunkExtent());

        // Send the tile update to all clients that are in range.
        std::vector<entt::entity>& entitiesInRange{
            world.entityLocator.getEntitiesFine(chunkExtent)};
        for (entt::entity entity : entitiesInRange) {
            ClientSimData& client{clientView.get<ClientSimData>(entity)};
            network.serializeAndSend<TileUpdate>(client.netID, tileUpdate);
            sentCount++;
        }
    }

    dirtyTiles.clear();
    double timeTaken{timer.getDeltaSeconds(false)};
    if (sentCount > 0) {
        LOG_INFO("Sent %u messages in %.6fs", sentCount, timeTaken);
    }
}

} // End namespace Server
} // End namespace AM
