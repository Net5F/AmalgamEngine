#include "TileMap.h"
#include "GraphicData.h"
#include "Sprite.h"
#include "Paths.h"
#include "Position.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "Tile.h"
#include "TileSnapshot.h"
#include "ChunkSnapshot.h"
#include "Morton.h"
#include "SharedConfig.h"
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
namespace Server
{
TileMap::TileMap(GraphicData& inGraphicData)
: TileMapBase{inGraphicData, true}
{
    // Prime a timer.
    Timer timer;

    // Deserialize the file into a snapshot.
    TileMapSnapshot mapSnapshot;
    std::string mapPath{Paths::BASE_PATH + "TileMap.bin"};
    bool loadSuccessful{Deserialize::fromFile(mapPath, mapSnapshot)};
    if (!loadSuccessful) {
        LOG_FATAL("Failed to deserialize map at path: %s", mapPath.c_str());
    }

    // Load the map snapshot.
    load(mapSnapshot);

    // Print the time taken.
    double timeTaken{timer.getTime()};
    LOG_INFO("Map loaded in %.6fs. Size: (%u, %u, %u)ch.", timeTaken,
             chunkExtent.xLength, chunkExtent.yLength, chunkExtent.zLength);
}

TileMap::~TileMap()
{
    // Save the map state to TileMap.bin.
    save("TileMap.bin");
}

void TileMap::save(const std::string& fileName)
{
    LOG_INFO("Saving map...");

    // Prime a timer.
    Timer timer{};

    /* Save this map's state into a snapshot. */
    // Save the header data.
    TileMapSnapshot mapSnapshot{};
    mapSnapshot.version = MAP_FORMAT_VERSION;
    mapSnapshot.xLengthChunks = static_cast<Uint16>(chunkExtent.xLength);
    mapSnapshot.yLengthChunks = static_cast<Uint16>(chunkExtent.yLength);
    mapSnapshot.zLengthChunks = static_cast<Uint16>(chunkExtent.zLength);

    // Save our tiles into the snapshot as chunks.
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    for (auto& [chunkPosition, chunk] : chunks) {
        ChunkSnapshot& chunkSnapshot{mapSnapshot.chunks[chunkPosition]};

        // Process each tile in this chunk.
        for (std::size_t i{0}; i < SharedConfig::CHUNK_TILE_COUNT;
             ++i) {
            // Copy all of this tile's layers into the snapshot.
            addTileLayersToSnapshot(chunk.tiles[i], chunkSnapshot.tiles[i],
                                    chunkSnapshot);
        }
    }

    // Serialize the map snapshot and write it to the file.
    bool saveSuccessful{
        Serialize::toFile((Paths::BASE_PATH + fileName), mapSnapshot)};
    if (saveSuccessful) {
        // Print the time taken.
        double timeTaken{timer.getTime()};
        LOG_INFO("Saved %u chunks in %.6fs.", chunkExtent.getCount(),
                 timeTaken);
    }
    else {
        LOG_FATAL("Failed to serialize and save the map.");
    }
}

void TileMap::load(TileMapSnapshot& mapSnapshot)
{
    /* Load the snapshot into this map. */
    // Load the header data.
    chunkExtent = ChunkExtent::fromMapLengths(mapSnapshot.xLengthChunks,
                                              mapSnapshot.yLengthChunks,
                                              mapSnapshot.zLengthChunks);
    tileExtent = TileExtent{chunkExtent};

    // Load all of the snapshot's chunks into our map.
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    for (auto& [chunkPosition, chunkSnapshot] : mapSnapshot.chunks) {
        int tileXOffset{chunkPosition.x * CHUNK_WIDTH};
        int tileYOffset{chunkPosition.y * CHUNK_WIDTH};

        // Iterate through the chunk snapshot's linear tile array, adding the 
        // tiles to our map.
        for (int tileX{0}; tileX < CHUNK_WIDTH; ++tileX) {
            for (int tileY{0}; tileY < CHUNK_WIDTH; ++tileY) {
                TilePosition tilePosition{tileX + tileXOffset,
                                          tileY + tileYOffset, chunkPosition.z};
                addSnapshotLayersToTile(
                    chunkSnapshot.tiles[Morton::m2D_lookup_16x16(tileX, tileY)],
                    chunkSnapshot, tilePosition);
            }
        }
    }
}

void TileMap::addTileLayersToSnapshot(const Tile& tile,
                                      TileSnapshot& tileSnapshot,
                                      ChunkSnapshot& chunkSnapshot)
{
    // Add all of the tile's layers.
    for (const TileLayer& layer : tile.getAllLayers()) {
        std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
            layer.type, layer.graphicSet.get().stringID, layer.graphicIndex)};
        tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
    }
}

} // End namespace Server
} // End namespace AM
