#include "TileMap.h"
#include "SpriteData.h"
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
#include "SharedConfig.h"
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"
#include "Ignore.h"

namespace AM
{
namespace Server
{
TileMap::TileMap(SpriteData& inSpriteData)
: TileMapBase{inSpriteData, true}
{
    // Prime a timer.
    Timer timer;

    // Deserialize the file into a snapshot.
    TileMapSnapshot mapSnapshot;
    std::string mapPath{Paths::BASE_PATH + "TileMap.bin"};
    if (!Deserialize::fromFile(mapPath, mapSnapshot)) {
        LOG_FATAL("Failed to deserialize map at path: %s", mapPath.c_str());
    }

    // Load the map snapshot.
    load(mapSnapshot);

    // Print the time taken.
    double timeTaken{timer.getTime()};
    LOG_INFO("Map loaded in %.6fs. Size: (%u, %u)ch.", timeTaken,
             chunkExtent.xLength, chunkExtent.yLength);
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
    Timer timer;

    /* Save this map's state into a snapshot. */
    // Save the header data.
    TileMapSnapshot mapSnapshot{};
    mapSnapshot.version = MAP_FORMAT_VERSION;
    mapSnapshot.xLengthChunks = chunkExtent.xLength;
    mapSnapshot.yLengthChunks = chunkExtent.yLength;

    // Allocate room for our chunks.
    mapSnapshot.chunks.resize(chunkExtent.getCount());

    // Save our tiles into the snapshot as chunks.
    std::size_t startLinearTileIndex{0};
    int chunksProcessed{0};
    for (std::size_t i = 0; i < chunkExtent.getCount(); ++i) {
        ChunkSnapshot& chunkSnapshot{mapSnapshot.chunks[i]};

        // Process each tile in this chunk.
        std::size_t nextLinearTileIndex{startLinearTileIndex};
        std::size_t tilesProcessed{0};
        for (std::size_t j = 0; j < SharedConfig::CHUNK_TILE_COUNT; ++j) {
            // Copy all of this tile's layers into the snapshot.
            addTileLayersToSnapshot(tiles[nextLinearTileIndex],
                                    chunkSnapshot.tiles[j], chunkSnapshot);

            // Increment to the next tile.
            nextLinearTileIndex++;

            // If we've processed all the tiles in this row, increment to the
            // next row.
            tilesProcessed++;
            if (tilesProcessed == SharedConfig::CHUNK_WIDTH) {
                nextLinearTileIndex
                    += (tileExtent.xLength - SharedConfig::CHUNK_WIDTH);
                tilesProcessed = 0;
            }
        }

        // Increment to the next chunk.
        startLinearTileIndex += SharedConfig::CHUNK_WIDTH;

        // If we've processed all the chunks in this row, increment to the
        // next row.
        chunksProcessed++;
        if (chunksProcessed == chunkExtent.xLength) {
            startLinearTileIndex
                += ((SharedConfig::CHUNK_WIDTH - 1) * tileExtent.xLength);
            chunksProcessed = 0;
        }
    }

    // Serialize the map snapshot and write it to the file.
    bool saveSuccessful{
        Serialize::toFile((Paths::BASE_PATH + fileName), mapSnapshot)};
    if (saveSuccessful) {
        // Print the time taken.
        double timeTaken{timer.getTime()};
        LOG_INFO("Map saved in %.6fs.", timeTaken);
    }
    else {
        LOG_FATAL("Failed to serialize and save the map.");
    }
}

void TileMap::load(TileMapSnapshot& mapSnapshot)
{
    /* Load the snapshot into this map. */
    // Load the header data.
    // Note: We set x/y to 0 since our map origin is always (0, 0). Change
    //       this if we ever support negative origins.
    chunkExtent.x = 0;
    chunkExtent.y = 0;
    chunkExtent.xLength = mapSnapshot.xLengthChunks;
    chunkExtent.yLength = mapSnapshot.yLengthChunks;
    tileExtent.x = 0;
    tileExtent.y = 0;
    tileExtent.xLength = (chunkExtent.xLength * SharedConfig::CHUNK_WIDTH);
    tileExtent.yLength = (chunkExtent.yLength * SharedConfig::CHUNK_WIDTH);

    // Resize the tiles vector to fit the map.
    tiles.resize(tileExtent.xLength * tileExtent.yLength);

    // Load the chunks into the tiles vector.
    for (std::size_t chunkIndex = 0; chunkIndex < mapSnapshot.chunks.size();
         ++chunkIndex) {
        // Calc the coordinates of this chunk's first tile.
        int startX{static_cast<int>((chunkIndex % chunkExtent.xLength)
                                    * SharedConfig::CHUNK_WIDTH)};
        int startY{static_cast<int>((chunkIndex / chunkExtent.xLength)
                                    * SharedConfig::CHUNK_WIDTH)};
        ChunkSnapshot& chunkSnapshot{mapSnapshot.chunks[chunkIndex]};

        // These vars track which tile we're looking at, with respect to the
        // top left of the chunk.
        int relativeX{0};
        int relativeY{0};

        // Add all of this chunk's tiles to the tiles vector.
        for (std::size_t i = 0; i < SharedConfig::CHUNK_TILE_COUNT; ++i) {
            // Push all of the snapshot's sprites into the tile.
            TileSnapshot& tileSnapshot{chunkSnapshot.tiles[i]};
            addSnapshotLayersToTile(tileSnapshot, chunkSnapshot,
                                    (startX + relativeX), (startY + relativeY));

            // Increment the relative indices, wrapping at the chunk width.
            relativeX++;
            if (relativeX == SharedConfig::CHUNK_WIDTH) {
                relativeY++;
                relativeX = 0;
            }
        }
    }
}

void TileMap::addTileLayersToSnapshot(const Tile& tile,
                                      TileSnapshot& tileSnapshot,
                                      ChunkSnapshot& chunkSnapshot)
{
    // Add the floor.
    std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
        TileLayer::Type::Floor, tile.getFloor().spriteSet->stringID, 0)};
    tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));

    // Add the floor coverings.
    const auto& floorCoverings{tile.getFloorCoverings()};
    for (const FloorCoveringTileLayer& floorCovering : floorCoverings) {
        std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
            TileLayer::Type::FloorCovering, floorCovering.spriteSet->stringID,
            floorCovering.direction)};
        tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
    }

    // Add the walls (skipping any empty elements).
    const auto& walls{tile.getWalls()};
    for (const WallTileLayer& wall : walls) {
        if (wall.wallType != Wall::Type::None) {
            std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
                TileLayer::Type::Wall, wall.spriteSet->stringID,
                wall.wallType)};
            tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
        }
    }

    // Add the objects.
    const auto& objects{tile.getObjects()};
    for (const ObjectTileLayer& object : objects) {
        std::size_t paletteIndex{chunkSnapshot.getPaletteIndex(
            TileLayer::Type::Object, object.spriteSet->stringID,
            object.direction)};
        tileSnapshot.layers.push_back(static_cast<Uint8>(paletteIndex));
    }
}

} // End namespace Server
} // End namespace AM
