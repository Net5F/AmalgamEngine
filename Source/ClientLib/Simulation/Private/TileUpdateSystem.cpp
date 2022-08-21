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
    TileUpdate tileUpdate{};
    while (tileUpdateQueue.pop(tileUpdate)) {
        // Clear the tile (the message contains all of the tile's layers).
        world.tileMap.clearTile(tileUpdate.tileX, tileUpdate.tileY);

        // Fill the tile with the layers from the message.
        for (unsigned int i = 0; i < tileUpdate.numericIDs.size(); ++i) {
            world.tileMap.setTileSpriteLayer(tileUpdate.tileX, tileUpdate.tileY,
                                             i, tileUpdate.numericIDs[i]);
            LOG_INFO("Setting (%d, %d) layer %u to %d", tileUpdate.tileX,
                     tileUpdate.tileY, i, tileUpdate.numericIDs[i]);
        }
    }
}

} // End namespace Client
} // End namespace AM
