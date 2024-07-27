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
    for (auto& [chunkPosition, chunk] : chunks) {
        ChunkSnapshot& chunkSnapshot{mapSnapshot.chunks[chunkPosition]};
        saveChunkToSnapshot(chunk, chunkSnapshot);
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

    // If the map is too big, exit.
    static constexpr int MAX_MAP_WIDTH_TILES{
        static_cast<int>(SharedConfig::MAX_MAP_WIDTH_TILES)};
    if ((tileExtent.xLength > MAX_MAP_WIDTH_TILES)
        || (tileExtent.yLength > MAX_MAP_WIDTH_TILES)
        || (tileExtent.zLength > MAX_MAP_WIDTH_TILES)) {
        LOG_FATAL("Failed to load map: Map width is larger than "
                  "MAX_MAP_WIDTH_TILES.");
    }

    // Load all of the snapshot's chunks into our map.
    for (auto& [chunkPosition, chunkSnapshot] : mapSnapshot.chunks) {
        loadChunk(chunkSnapshot,
                  {chunkPosition.x, chunkPosition.y, chunkPosition.z});
    }
}

void TileMap::saveChunkToSnapshot(const Chunk& chunk,
                                  ChunkSnapshot& chunkSnapshot)
{
    // Copy all of the chunk's tile layers into the snapshot.
    chunkSnapshot.tileLayers.resize(chunk.tileLayerCount);
    std::size_t tileLayersIndex{0};
    for (std::size_t tileIndex{0};
         tileIndex < SharedConfig::CHUNK_TILE_COUNT; tileIndex++) {
        // Add this tile's layer count.
        const Tile& tile{chunk.tiles[tileIndex]};
        chunkSnapshot.tileLayerCounts[tileIndex]
            = static_cast<Uint8>(tile.getAllLayers().size());

        // Add all of this tile's layers.
        for (const TileLayer& layer : tile.getAllLayers()) {
            std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
                layer.type, layer.graphicSet.get().stringID,
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

} // End namespace Server
} // End namespace AM
