#include "TileUpdateSystem.h"
#include "World.h"
#include "Network.h"

namespace AM
{
namespace Client
{
TileUpdateSystem::TileUpdateSystem(World& inWorld,
                                   EventDispatcher& inUiEventDispatcher,
                                   Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, tileUpdateRequestQueue(inUiEventDispatcher)
, tileUpdateQueue(network.getEventDispatcher())
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
    TileUpdateRequest uiRequest;
    while (tileUpdateRequestQueue.pop(uiRequest)) {
        // Send the request to the server.
        network.serializeAndSend<TileUpdateRequest>(uiRequest);
    }
}

void TileUpdateSystem::processNetworkUpdates()
{
    // Process any waiting tile updates from the server.
    TileUpdate tileUpdate;
    while (tileUpdateQueue.pop(tileUpdate)) {
        // Update the map.
        world.tileMap.setTileSpriteLayer(tileUpdate.tileX, tileUpdate.tileY,
                                         tileUpdate.layerIndex,
                                         tileUpdate.numericID);
    }
}

} // End namespace Client
} // End namespace AM
