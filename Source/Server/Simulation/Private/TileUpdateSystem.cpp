#include "TileUpdateSystem.h"
#include "World.h"
#include "Network.h"
#include "ClientSimData.h"
#include "TileUpdate.h"

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
    auto clientView = world.registry.view<ClientSimData>();

    // Process any waiting update requests.
    TileUpdateRequest updateRequest;
    while (tileUpdateRequestQueue.pop(updateRequest)) {
        // Update the map.
        // Note: This doesn't check if the client entity is within any certain
        //       range of the tile or anything. We can add that if it's
        //       useful, I just couldn't immediately think of a use case for it.
        world.tileMap.setTileSpriteLayer(updateRequest.tileX, updateRequest.tileY,
                                     updateRequest.layerIndex,
                                     updateRequest.numericID);

        // Construct the new tile update.
        TileUpdate tileUpdate{updateRequest.tileX, updateRequest.tileY,
                              updateRequest.layerIndex,
                              updateRequest.numericID};

        // Get the list of clients that are in range of the updated tile.
        // Note: This is hardcoded to match ChunkUpdateSystem.
        ChunkPosition centerChunk{
            TilePosition{updateRequest.tileX, updateRequest.tileY}};
        ChunkExtent chunkExtent{(centerChunk.x - 1), (centerChunk.y - 1), 3, 3};
        chunkExtent.intersectWith(world.tileMap.getChunkExtent());

        // Send the tile update to all clients that are in range.
        std::vector<entt::entity>& entitiesInRange{
            world.entityLocator.getEntitiesFine(chunkExtent)};
        for (entt::entity entity : entitiesInRange) {
            ClientSimData& client{clientView.get<ClientSimData>(entity)};
            network.serializeAndSend<TileUpdate>(client.netID, tileUpdate);
        }
    }
}

} // End namespace Server
} // End namespace AM
