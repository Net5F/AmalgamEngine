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
    const int CHUNK_WIDTH{static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    // Push the new chunk and get a ref to it.
    chunkUpdate.chunks.emplace_back();
    ChunkWireSnapshot& chunkSnapshot{chunkUpdate.chunks.back()};

    // Save the chunk's position.
    chunkSnapshot.x = chunkPosition.x;
    chunkSnapshot.y = chunkPosition.y;

    // Calc what the chunk's starting tile is.
    int startX{chunkSnapshot.x * CHUNK_WIDTH};
    int startY{chunkSnapshot.y * CHUNK_WIDTH};

    // For each tile in the chunk.
    int tileIndex{0};
    for (int tileY = 0; tileY < CHUNK_WIDTH; ++tileY) {
        for (int tileX = 0; tileX < CHUNK_WIDTH; ++tileX) {
            // Copy all of this tile's layers to the snapshot.
            const Tile& tile{
                world.tileMap.getTile((startX + tileX), (startY + tileY))};
            addTileLayersToSnapshot(tile, chunkSnapshot.tiles[tileIndex],
                                    chunkSnapshot);

            // Increment to the next linear index.
            tileIndex++;
        }
    }
}

void ChunkStreamingSystem::addTileLayersToSnapshot(
    const Tile& tile, TileSnapshot& tileSnapshot,
    ChunkWireSnapshot& chunkSnapshot)
{
    // Add the floor (if one is present).
    const FloorGraphicSet* graphicSet{tile.getFloor().graphicSet};
    if (graphicSet) {
        std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
            TileLayer::Type::Floor, graphicSet->numericID, 0)};
        tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
    }

    // Add the floor coverings.
    const auto& floorCoverings{tile.getFloorCoverings()};
    for (const FloorCoveringTileLayer& floorCovering : floorCoverings) {
        std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
            TileLayer::Type::FloorCovering, floorCovering.graphicSet->numericID,
            floorCovering.direction)};
        tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
    }

    // Add the walls (skipping any empty elements).
    const auto& walls{tile.getWalls()};
    for (const WallTileLayer& wall : walls) {
        if (wall.wallType != Wall::Type::None) {
            std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
                TileLayer::Type::Wall, wall.graphicSet->numericID,
                wall.wallType)};
            tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
        }
    }

    // Add the objects.
    const auto& objects{tile.getObjects()};
    for (const ObjectTileLayer& object : objects) {
        std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
            TileLayer::Type::Object, object.graphicSet->numericID,
            object.direction)};
        tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
    }
}

} // End namespace Server
} // End namespace AM
