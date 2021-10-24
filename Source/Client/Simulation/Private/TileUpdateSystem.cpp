#include "TileUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"

namespace AM
{
namespace Client
{
TileUpdateSystem::TileUpdateSystem(World& inWorld,
                                   EventDispatcher& inUiEventDispatcher,
                                   EventDispatcher& inNetworkEventDispatcher,
                                   Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, tileUpdateRequestQueue(inUiEventDispatcher)
, tileUpdateQueue(inNetworkEventDispatcher)
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
        // Update the map locally.
        // Note: This assumes that the server will accept the request.
        world.tileMap.setSpriteLayer(uiRequest.tileX, uiRequest.tileY,
                                     uiRequest.layerIndex, uiRequest.numericID);

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
        world.tileMap.setSpriteLayer(tileUpdate.tileX, tileUpdate.tileY,
                                     tileUpdate.layerIndex,
                                     tileUpdate.numericID);
    }
}

} // End namespace Client
} // End namespace AM
