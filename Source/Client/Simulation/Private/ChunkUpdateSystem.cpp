#include "ChunkUpdateSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "NeedsAdjacentChunks.h"
#include "ChunkPosition.h"
#include "ChunkRange.h"
#include "ChunkUpdateRequest.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Client
{
ChunkUpdateSystem::ChunkUpdateSystem(Simulation& inSim, World& inWorld,
                                     EventDispatcher& inNetworkEventDispatcher,
                                     Network& inNetwork)
: sim{inSim}
, world{inWorld}
, network{inNetwork}
, chunkUpdateQueue{inNetworkEventDispatcher}
{
}

void ChunkUpdateSystem::updateChunks()
{
    // Request chunk updates, if necessary.
    requestNeededUpdates();

    // Process any received chunk updates.
    receiveAndApplyUpdates();
}

void ChunkUpdateSystem::requestNeededUpdates()
{
    entt::registry& registry = world.registry;
    Position& currentPosition = registry.get<Position>(world.playerEntity);
    PreviousPosition& previousPosition
        = registry.get<PreviousPosition>(world.playerEntity);

    // If we're flagged as needing to load all adjacent chunks, request them.
    if (registry.all_of<NeedsAdjacentChunks>(world.playerEntity)) {
        requestAllInRangeChunks(currentPosition.asChunkPosition());

        registry.remove<NeedsAdjacentChunks>(world.playerEntity);
    }
    // If we moved, check if we're in range of any new chunks.
    else if (previousPosition != currentPosition) {
        // If we moved into a new chunk.
        ChunkPosition previousChunk{previousPosition.asChunkPosition()};
        ChunkPosition currentChunk{currentPosition.asChunkPosition()};
        if (previousChunk != currentChunk) {
            // Request the chunks that we're now in range of.
            requestNewInRangeChunks(previousChunk, currentChunk);
        }
    }
}

void ChunkUpdateSystem::requestAllInRangeChunks(
    const ChunkPosition& currentChunk)
{
    // Determine which chunks are in range of each chunk position.
    // Note: This is hardcoded to assume the range is all chunks directly
    //       surrounding a given chunk.
    ChunkRange currentRange{(currentChunk.x - 1), (currentChunk.y - 1), 3, 3};

    // Bound the range to the map boundaries.
    ChunkRange mapBounds{0, 0, static_cast<int>(world.tileMap.xLengthChunks()),
                         static_cast<int>(world.tileMap.yLengthChunks())};
    currentRange.intersectWith(mapBounds);

    // Iterate over the current range, adding any new chunks to a request.
    ChunkUpdateRequest chunkUpdateRequest{};
    for (int i = 0; i < currentRange.yLength; ++i) {
        for (int j = 0; j < currentRange.xLength; ++j) {
            // If this chunk isn't in range of the previous chunk, add it.
            int chunkX{currentRange.x + j};
            int chunkY{currentRange.y + i};

            chunkUpdateRequest.requestedChunks.emplace_back(chunkX, chunkY);
        }
    }

    // Send the request.
    network.serializeAndSend(chunkUpdateRequest);
}

void ChunkUpdateSystem::requestNewInRangeChunks(
    const ChunkPosition& previousChunk, const ChunkPosition& currentChunk)
{
    // Determine which chunks are in range of each chunk position.
    // Note: This is hardcoded to assume the range is all chunks directly
    //       surrounding a given chunk.
    ChunkRange previousRange{(previousChunk.x - 1), (previousChunk.y - 1), 3,
                             3};
    ChunkRange currentRange{(currentChunk.x - 1), (currentChunk.y - 1), 3, 3};

    // Bound each range to the map boundaries.
    ChunkRange mapBounds{0, 0, static_cast<int>(world.tileMap.xLengthChunks()),
                         static_cast<int>(world.tileMap.yLengthChunks())};
    previousRange.intersectWith(mapBounds);
    currentRange.intersectWith(mapBounds);

    // Iterate over the current range, adding any new chunks to a request.
    ChunkUpdateRequest chunkUpdateRequest;
    for (int i = 0; i < currentRange.yLength; ++i) {
        for (int j = 0; j < currentRange.xLength; ++j) {
            // If this chunk isn't in range of the previous chunk, add it.
            int chunkX{currentRange.x + j};
            int chunkY{currentRange.y + i};
            ChunkPosition chunkPosition{chunkX, chunkY};

            if (!(previousRange.containsPosition(chunkPosition))) {
                chunkUpdateRequest.requestedChunks.emplace_back(chunkX, chunkY);
            }
        }
    }

    // Send the request.
    network.serializeAndSend(chunkUpdateRequest);
}

void ChunkUpdateSystem::receiveAndApplyUpdates()
{
    // Process any received chunk updates.
    std::shared_ptr<const ChunkUpdate> receivedUpdate{nullptr};
    while (chunkUpdateQueue.pop(receivedUpdate)) {
        // Apply all chunk snapshots from the update to our map.
        for (const ChunkWireSnapshot& chunk : receivedUpdate->chunks) {
            applyChunkSnapshot(chunk);
        }
    }
}

void ChunkUpdateSystem::applyChunkSnapshot(const ChunkWireSnapshot& chunk)
{
    // Iterate through the chunk snapshot's linear tile array, adding the tiles
    // to our map.
    int tileIndex{0};
    for (unsigned int tileY = 0; tileY < SharedConfig::CHUNK_WIDTH; ++tileY) {
        for (unsigned int tileX = 0; tileX < SharedConfig::CHUNK_WIDTH;
             ++tileX) {
            // Calculate where this tile is.
            unsigned int currentTileX{
                ((chunk.x * SharedConfig::CHUNK_WIDTH) + tileX)};
            unsigned int currentTileY{
                ((chunk.y * SharedConfig::CHUNK_WIDTH) + tileY)};

            // Clear the tile.
            world.tileMap.clearTile(currentTileX, currentTileY);

            // Copy all of the snapshot tile's sprite layers to our map tile.
            const TileSnapshot& tileSnapshot = chunk.tiles[tileIndex];
            unsigned int layerIndex{0};
            for (Uint8 paletteID : tileSnapshot.spriteLayers) {
                // Add the sprite layer to the tile.
                world.tileMap.setSpriteLayer(currentTileX, currentTileY,
                                             layerIndex++,
                                             chunk.palette[paletteID]);
            }

            // Increment to the next linear index.
            tileIndex++;
        }
    }
}

} // namespace Client
} // namespace AM
