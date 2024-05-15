#include "ChunkUpdateSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Network.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "NeedsAdjacentChunks.h"
#include "ChunkExtent.h"
#include "ChunkDataRequest.h"
#include "ChunkWireSnapshot.h"
#include "Morton.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Client
{
ChunkUpdateSystem::ChunkUpdateSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, chunkUpdateQueue{network.getEventDispatcher()}
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
    entt::registry& registry{world.registry};
    Position& currentPosition{registry.get<Position>(world.playerEntity)};
    PreviousPosition& previousPosition{
        registry.get<PreviousPosition>(world.playerEntity)};

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
    // Determine which chunks are in range of the given position.
    // Note: This is hardcoded to assume the range is all chunks directly
    //       surrounding a given chunk.
    ChunkExtent currentExtent{(currentChunk.x - 1),
                              (currentChunk.y - 1),
                              (currentChunk.z - 1),
                              3,
                              3,
                              3};

    // Bound the range to the map boundaries.
    const ChunkExtent& mapChunkExtent{world.tileMap.getChunkExtent()};
    currentExtent.intersectWith(mapChunkExtent);

    // Iterate over the range, adding all chunks to a request.
    ChunkDataRequest chunkDataRequest{};
    for (int i{0}; i < currentExtent.zLength; ++i) {
        for (int j{0}; j < currentExtent.yLength; ++j) {
            for (int k{0}; k < currentExtent.xLength; ++k) {
                int chunkX{currentExtent.x + k};
                int chunkY{currentExtent.y + j};
                int chunkZ{currentExtent.z + i};
                chunkDataRequest.requestedChunks.emplace_back(chunkX, chunkY,
                                                              chunkZ);
            }
        }
    }

    // Send the request.
    network.serializeAndSend(chunkDataRequest);
}

void ChunkUpdateSystem::requestNewInRangeChunks(
    const ChunkPosition& previousChunk, const ChunkPosition& currentChunk)
{
    // Determine which chunks are in range of each chunk position.
    // Note: This is hardcoded to assume the range is all chunks directly
    //       surrounding a given chunk.
    ChunkExtent previousExtent{(previousChunk.x - 1),
                               (previousChunk.y - 1),
                               (previousChunk.z - 1),
                               3,
                               3,
                               3};
    ChunkExtent currentExtent{(currentChunk.x - 1),
                              (currentChunk.y - 1),
                              (currentChunk.z - 1),
                              3,
                              3,
                              3};

    // Bound each range to the map boundaries.
    const ChunkExtent& mapChunkExtent{world.tileMap.getChunkExtent()};
    previousExtent.intersectWith(mapChunkExtent);
    currentExtent.intersectWith(mapChunkExtent);

    // Iterate over the current extent, adding any new chunks to a request.
    ChunkDataRequest chunkDataRequest;
    for (int i{0}; i < currentExtent.zLength; ++i) {
        for (int j{0}; j < currentExtent.yLength; ++j) {
            for (int k{0}; k < currentExtent.xLength; ++k) {
                // If this chunk isn't in range of the previous chunk, add it.
                int chunkX{currentExtent.x + k};
                int chunkY{currentExtent.y + j};
                int chunkZ{currentExtent.z + i};
                ChunkPosition chunkPosition{chunkX, chunkY, chunkZ};

                if (!(previousExtent.containsPosition(chunkPosition))) {
                    chunkDataRequest.requestedChunks.emplace_back(
                        chunkX, chunkY, chunkZ);
                }
            }
        }
    }

    // Send the request.
    network.serializeAndSend(chunkDataRequest);
}

void ChunkUpdateSystem::receiveAndApplyUpdates()
{
    // Disable auto collision rebuild (it's more efficient to do it all after).
    world.tileMap.setAutoRebuildCollision(false);

    // Process any received chunk updates.
    std::shared_ptr<const ChunkUpdate> receivedUpdate{nullptr};
    while (chunkUpdateQueue.pop(receivedUpdate)) {
        // Apply all chunk snapshots from the update to our map.
        for (const ChunkWireSnapshot& chunk : receivedUpdate->chunks) {
            applyChunkSnapshot(chunk);
        }
    }

    // Re-enable auto collision rebuild (rebuilds any dirty tiles).
    world.tileMap.setAutoRebuildCollision(true);
}

void ChunkUpdateSystem::applyChunkSnapshot(
    const ChunkWireSnapshot& chunkSnapshot)
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    // Iterate through the chunk snapshot's linear tile array, adding the tiles
    // to our map.
    for (int tileY{0}; tileY < CHUNK_WIDTH; ++tileY) {
        for (int tileX{0}; tileX < CHUNK_WIDTH; ++tileX) {
            // Calculate where this tile is.
            TilePosition tilePosition{tileX, tileY, chunkSnapshot.z};
            tilePosition.x += (chunkSnapshot.x * CHUNK_WIDTH);
            tilePosition.y += (chunkSnapshot.y * CHUNK_WIDTH);

            // Clear the tile.
            world.tileMap.clearTile(tilePosition);

            // Copy all of the snapshot tile's sprite layers to our map
            // tile.
            const TileSnapshot& tileSnapshot{
                chunkSnapshot.tiles[Morton::m2D_lookup_16x16(tileX, tileY)]};
            world.tileMap.addSnapshotLayersToTile(
                tileSnapshot, chunkSnapshot, tilePosition);
        }
    }
}

} // namespace Client
} // namespace AM
