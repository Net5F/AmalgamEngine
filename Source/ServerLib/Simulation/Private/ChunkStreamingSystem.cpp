#include "ChunkStreamingSystem.h"
#include "World.h"
#include "Network.h"
#include "ClientSimData.h"
#include "Sprite.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "ChunkUpdate.h"
#include "Tile.h"
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
        chunkSnapshot.tileLayers.resize(chunk->tileLayerCount);
        std::size_t tileLayersIndex{0};
        for (std::size_t tileIndex{0};
             tileIndex < SharedConfig::CHUNK_TILE_COUNT; tileIndex++) {
            // Add this tile's layer count.
            const Tile& tile{chunk->tiles[tileIndex]};
            chunkSnapshot.tileLayerCounts[tileIndex]
                = static_cast<Uint8>(tile.getAllLayers().size());

            // Add all of this tile's layers.
            for (const TileLayer& layer : tile.getAllLayers()) {
                std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
                    layer.type, layer.graphicSet.get().numericID,
                    layer.graphicValue)};
                chunkSnapshot.tileLayers[tileLayersIndex]
                    = static_cast<Uint8>(paletteIndex);
                tileLayersIndex++;

                // If this is a Floor or Object, add its tile offset.
                if ((layer.type == TileLayer::Type::Floor)
                    || (layer.type == TileLayer::Type::Object)) {
                    chunkSnapshot.tileOffsets.emplace_back(layer.tileOffset);
                }
            }
        }
    }
    else {
        // This chunk doesn't exist, we don't need to send anything.
    }
}

} // End namespace Server
} // End namespace AM
