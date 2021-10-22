#include "ChunkStreamingSystem.h"
#include "World.h"
#include "Network.h"
#include "ClientSimData.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "ChunkPosition.h"
#include "ChunkRange.h"
#include "ChunkUpdate.h"
#include "SharedConfig.h"
#include "Serialize.h"
#include "Log.h"
#include <SDL2/SDL_rect.h>
#include <vector>

namespace AM
{
namespace Server
{
ChunkStreamingSystem::ChunkStreamingSystem(World& inWorld, EventDispatcher& inNetworkEventDispatcher, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, chunkUpdateRequestQueue(inNetworkEventDispatcher)
{
}

void ChunkStreamingSystem::sendChunks()
{
    // Process all chunk update requests.
    ChunkUpdateRequest chunkUpdateRequest{};
    while (chunkUpdateRequestQueue.pop(chunkUpdateRequest)) {
        sendChunkUpdate(chunkUpdateRequest);
    }
}

void ChunkStreamingSystem::sendChunkUpdate(
    const ChunkUpdateRequest& chunkUpdateRequest)
{
    // Add the requested chunks to the message.
    ChunkUpdate chunkUpdate{};
    for (const ChunkPosition& requestedChunk : chunkUpdateRequest.requestedChunks) {
        addChunkToMessage(requestedChunk, chunkUpdate);
    }

    // Send the message.
    network.serializeAndSend(chunkUpdateRequest.netID, chunkUpdate);
}

void ChunkStreamingSystem::addChunkToMessage(const ChunkPosition& chunkPosition,
                                             ChunkUpdate& chunkUpdate)
{
    // Push the new chunk and get a ref to it.
    chunkUpdate.chunks.emplace_back();
    ChunkWireSnapshot& chunk{chunkUpdate.chunks.back()};

    // Save the chunk's position.
    chunk.x = chunkPosition.x;
    chunk.y = chunkPosition.y;

    // Calc what the chunk's starting tile is.
    unsigned int startX{chunk.x * SharedConfig::CHUNK_WIDTH};
    unsigned int startY{chunk.y * SharedConfig::CHUNK_WIDTH};

    // For each tile in the chunk.
    int tileIndex{0};
    for (unsigned int tileY = 0; tileY < SharedConfig::CHUNK_WIDTH; ++tileY) {
        for (unsigned int tileX = 0; tileX < SharedConfig::CHUNK_WIDTH; ++tileX) {
            // Copy all of the tile's layers to the snapshot.
            const Tile& tile = world.tileMap.getTile((startX + tileX), (startY + tileY));
            for (const Tile::SpriteLayer& layer : tile.spriteLayers) {
                unsigned int paletteID{
                    chunk.getPaletteIndex(layer.sprite->numericID)};
                chunk.tiles[tileIndex].spriteLayers.push_back(paletteID);
            }

            // Increment to the next linear index.
            tileIndex++;
        }
    }
}

} // End namespace Server
} // End namespace AM
