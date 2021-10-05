#include "ChunkUpdateSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "Position.h"
#include "Movement.h"
#include "Input.h"
#include "PlayerState.h"
#include "PreviousPosition.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Client
{
ChunkUpdateSystem::ChunkUpdateSystem(Simulation& inSim, World& inWorld,
                                           Network& inNetwork, SpriteData& inSpriteData)
: sim{inSim}
, tileMap{inWorld.tileMap}
, spriteData{inSpriteData}
, chunkUpdateQueue{inNetwork.getDispatcher()}
{
}

void ChunkUpdateSystem::updateChunks()
{
    // Process any received chunk updates.
    std::shared_ptr<const ChunkUpdate> receivedUpdate{nullptr};
    while (chunkUpdateQueue.pop(receivedUpdate)) {
        // Apply all chunk snapshots from the update to our map.
        for (const ChunkWireSnapshot& chunk : receivedUpdate->chunks) {
            LOG_INFO("Adding chunk (%u, %u)", chunk.x, chunk.y);
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
        for (unsigned int tileX = 0; tileX < SharedConfig::CHUNK_WIDTH; ++tileX) {
            // Calculate where this tile is.
            unsigned int currentTileX{((chunk.x * SharedConfig::CHUNK_WIDTH) + tileX)};
            unsigned int currentTileY{((chunk.y * SharedConfig::CHUNK_WIDTH) + tileY)};

            // Clear the tile.
            tileMap.clearTile(currentTileX, currentTileY);

            // Copy all of the snapshot tile's sprite layers to our map tile.
            const TileSnapshot& tileSnapshot = chunk.tiles[tileIndex];
            for (Uint8 paletteID : tileSnapshot.spriteLayers) {
                // Get the sprite that this palette ID is referring to.
                const Sprite& sprite{spriteData.get(chunk.palette[paletteID])};

                // Add the sprite layer to the tile.
                tileMap.addSpriteLayer(currentTileX, currentTileY, sprite);
            }

            // Increment to the next linear index.
            tileIndex++;
        }
    }
}

} // namespace Client
} // namespace AM
