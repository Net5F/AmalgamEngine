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
    // Process any waiting update requests.
    TileUpdateRequest updateRequest;
    while (tileUpdateRequestQueue.pop(updateRequest)) {
        // Update the map.
        // Note: This doesn't check if the client entity is within any certain
        //       range of the tile or anything. We can add that if it's
        //       useful, I just couldn't immediately think of a use case for it.
        world.tileMap.setSpriteLayer(updateRequest.tileX, updateRequest.tileY,
                                     updateRequest.layerIndex,
                                     updateRequest.numericID);

        // Construct the new tile update.
        TileUpdate tileUpdate{updateRequest.tileX, updateRequest.tileY,
                              updateRequest.layerIndex,
                              updateRequest.numericID};

        // TODO: Limit this to entities that are in range.
        // Broadcast the tile update to all clients.
        auto clientView = world.registry.view<ClientSimData>();
        for (entt::entity entity : clientView) {
            ClientSimData& client{clientView.get<ClientSimData>(entity)};
            network.serializeAndSend<TileUpdate>(client.netID, tileUpdate);
        }
    }
}

} // End namespace Server
} // End namespace AM
