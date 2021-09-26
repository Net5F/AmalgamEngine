#include "ChunkStreamingSystem.h"
#include "World.h"
#include "Network.h"
#include "ClientSimData.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "ChunkIndex.h"
#include "ChunkRange.h"
#include "UpdateChunks.h"
#include "SharedConfig.h"
#include "Serialize.h"
#include "Log.h"
#include "Ignore.h"
#include <SDL2/SDL_rect.h>
#include <vector>

namespace AM
{
namespace Server
{
ChunkStreamingSystem::ChunkStreamingSystem(World& inWorld, Network& inNetwork)
: registry{inWorld.registry}
, tileMap{inWorld.tileMap}
, network{inNetwork}
{
    // Init the groups that we'll be using.
    auto clientGroup
        = registry.group<ClientSimData>(entt::get<Position, PreviousPosition>);
    ignore(clientGroup);
}

void ChunkStreamingSystem::sendChunks()
{
    // Iterate through the clients, checking if they need to be sent map data.
    auto clientGroup
        = registry.group<ClientSimData>(entt::get<Position, PreviousPosition>);
    for (entt::entity entity : clientGroup) {
        auto [client, position, previousPosition]
            = clientGroup.get<ClientSimData, Position, PreviousPosition>(
                entity);

        // If the client just joined and needs all their chunks.
        if (client.needsInitialChunks) {
            sendAllInRangeChunks(position.asChunkIndex(), client.netID);
            client.needsInitialChunks = false;
        }
        // Else if the client moved on this tick.
        else if (position != previousPosition) {
            LOG_INFO("Check: (%.4f, %.4f), (%.4f, %.4f)", previousPosition.x,
                     previousPosition.y, position.x, position.y);
            // If they moved into a new chunk.
            ChunkIndex previousChunk{previousPosition.asChunkIndex()};
            ChunkIndex currentChunk{position.asChunkIndex()};
            if (previousChunk != currentChunk) {
                // Send the chunks that they're now in range of.
                sendNewInRangeChunks(previousChunk, currentChunk, client.netID);
            }
        }
    }
}

void ChunkStreamingSystem::sendAllInRangeChunks(const ChunkIndex& currentChunk,
                                                NetworkID netID)
{
    // Sends all chunks in range of the current chunk.
    // Note: This is hardcoded to assume the range is all chunks directly
    //       surrounding the given chunk.
    ChunkRange currentRange{(currentChunk.x - 1), (currentChunk.y - 1), 3, 3};

    // Bound the range to the map boundaries.
    ChunkRange mapBounds{0, 0, static_cast<int>(tileMap.xLengthChunks()),
                         static_cast<int>(tileMap.yLengthChunks())};
    currentRange.setToIntersect(mapBounds);

    // Build the chunk update message.
    UpdateChunks updateChunks;
    for (int i = 0; i < currentRange.yLength; ++i) {
        for (int j = 0; j < currentRange.xLength; ++j) {
            // Add the chunk to the message.
            ChunkIndex chunkIndex{(currentRange.x + j), (currentRange.y + i)};
            addChunkToMessage(chunkIndex, updateChunks);
        }
    }

    // Send the chunk update message.
    network.serializeAndSend(netID, updateChunks);
    LOG_INFO("Sent initial UpdateChunks.");
}

void ChunkStreamingSystem::sendNewInRangeChunks(const ChunkIndex& previousChunk,
                                                const ChunkIndex& currentChunk,
                                                NetworkID netID)
{
    // Determine what chunks are in range of each chunk.
    // Note: This is hardcoded to assume the range is all chunks directly
    //       surrounding the given chunk.
    ChunkRange previousRange{(previousChunk.x - 1), (previousChunk.y - 1), 3,
                             3};
    ChunkRange currentRange{(currentChunk.x - 1), (currentChunk.y - 1), 3, 3};

    // Bound each range to the map boundaries.
    ChunkRange mapBounds{0, 0, static_cast<int>(tileMap.xLengthChunks()),
                         static_cast<int>(tileMap.yLengthChunks())};
    previousRange.setToIntersect(mapBounds);
    currentRange.setToIntersect(mapBounds);

    // Build the chunk update message.
    UpdateChunks updateChunks;
    for (int i = 0; i < currentRange.yLength; ++i) {
        for (int j = 0; j < currentRange.xLength; ++j) {
            // Skip the current chunk.
            if (i == 1 && j == 1) {
                continue;
            }

            // If this chunk isn't in range of the previous chunk, add it.
            ChunkIndex chunkIndex{(currentRange.x + j), (currentRange.y + i)};
            if (!(previousRange.containsIndex(chunkIndex))) {
                addChunkToMessage(chunkIndex, updateChunks);
            }
        }
    }

    // Send the chunk update message.
    network.serializeAndSend(netID, updateChunks);
    LOG_INFO("Sent UpdateChunks.");
}

void ChunkStreamingSystem::addChunkToMessage(const ChunkIndex& chunkIndex,
                                             UpdateChunks& updateChunks)
{
    // Push the new chunk and get a ref to it.
    updateChunks.chunks.emplace_back();
    ChunkWireSnapshot& chunk{updateChunks.chunks.back()};

    // Calc what the chunk's starting tile is.
    unsigned int startX{chunkIndex.x * SharedConfig::CHUNK_WIDTH};
    unsigned int startY{chunkIndex.y * SharedConfig::CHUNK_WIDTH};

    // For each tile in the chunk.
    int tileIndex{0};
    for (unsigned int y = 0; y < SharedConfig::CHUNK_WIDTH; ++y) {
        for (unsigned int x = 0; x < SharedConfig::CHUNK_WIDTH; ++x) {
            // Copy all of the tile's layers to the snapshot.
            const Tile& tile{tileMap.getTile((startX + x), (startY + y))};
            for (const Tile::SpriteLayer& layer : tile.spriteLayers) {
                unsigned int paletteId{
                    chunk.getPaletteIndex(layer.sprite->numericId)};
                chunk.tiles[tileIndex].spriteLayers.push_back(paletteId);
            }

            // Increment to the next linear index.
            tileIndex++;
        }
    }
}

} // End namespace Server
} // End namespace AM
