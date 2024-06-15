#include "ChunkStreamingSystem.h"
#include "World.h"
#include "Network.h"
#include "ClientSimData.h"
#include "Sprite.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "ChunkUpdate.h"
#include "Tile.h"
#include "TileSnapshot.h"
#include "ChunkWireSnapshot.h"
#include "SharedConfig.h"
#include "Log.h"
#include <SDL_rect.h>
#include "tracy/Tracy.hpp"
#include <vector>

namespace AM
{
namespace Server
{
ChunkStreamingSystem::ChunkStreamingSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, chunkDataRequestQueue{inNetwork.getEventDispatcher()}
{
}

void ChunkStreamingSystem::sendChunks()
{
    ZoneScoped;

    // Process all chunk data requests.
    ChunkDataRequest chunkDataRequest{};
    while (chunkDataRequestQueue.pop(chunkDataRequest)) {
        sendChunkUpdate(chunkDataRequest);
    }
}

void ChunkStreamingSystem::sendChunkUpdate(
    const ChunkDataRequest& chunkDataRequest)
{
    // Add the requested chunks to the message.
    ChunkUpdate chunkUpdate{};
    for (const ChunkPosition& requestedChunk :
         chunkDataRequest.requestedChunks) {
        addChunkToMessage(requestedChunk, chunkUpdate);
    }

    // Send the message.
    network.serializeAndSend(chunkDataRequest.netID, chunkUpdate);
}

void ChunkStreamingSystem::addChunkToMessage(const ChunkPosition& chunkPosition,
                                             ChunkUpdate& chunkUpdate)
{
    if (const Chunk* chunk{world.tileMap.cgetChunk(chunkPosition)}) {
        // Push the new chunk and get a ref to it.
        chunkUpdate.chunks.emplace_back();
        ChunkWireSnapshot& chunkSnapshot{chunkUpdate.chunks.back()};

        // Save the chunk's position.
        chunkSnapshot.x = chunkPosition.x;
        chunkSnapshot.y = chunkPosition.y;
        chunkSnapshot.z = chunkPosition.z;

        // Copy all of the chunk's tile layers into the snapshot.
        for (std::size_t tileIndex{0};
             tileIndex < SharedConfig::CHUNK_TILE_COUNT; tileIndex++) {
            addTileLayersToSnapshot(chunk->tiles[tileIndex],
                                    chunkSnapshot.tiles[tileIndex],
                                    chunkSnapshot);
        }
    }
    else {
        // This chunk doesn't exist, we don't need to send anything.
    }
}

void ChunkStreamingSystem::addTileLayersToSnapshot(
    const Tile& tile, TileSnapshot& tileSnapshot,
    ChunkWireSnapshot& chunkSnapshot)
{
    // Add all of the tile's layers.
    for (const TileLayer& layer : tile.getAllLayers()) {
        std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
            layer.type, layer.graphicSet.get().numericID, layer.graphicValue)};
        tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
    }
}

} // End namespace Server
} // End namespace AM
