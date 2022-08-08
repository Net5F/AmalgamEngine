#include "TileMap.h"
#include "SpriteData.h"
#include "Paths.h"
#include "Position.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
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
: TileMapBase{inSpriteData}
{
    // Prime a timer.
    Timer timer;
    timer.updateSavedTime();

    // Deserialize the file into a snapshot.
    TileMapSnapshot mapSnapshot;
    Deserialize::fromFile((Paths::BASE_PATH + "TileMap.bin"), mapSnapshot);

    // Load the map snapshot.
    load(mapSnapshot);

    // Print the time taken.
    double timeTaken{timer.getDeltaSeconds(false)};
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
    // Prime a timer.
    Timer timer;
    timer.updateSavedTime();

    /* Save this map's state into a snapshot. */
    // Save the header data.
    TileMapSnapshot mapSnapshot{};
    mapSnapshot.version = MAP_FORMAT_VERSION;
    mapSnapshot.xLengthChunks = chunkExtent.xLength;
    mapSnapshot.yLengthChunks = chunkExtent.yLength;

    // Allocate room for our chunks.
    mapSnapshot.chunks.resize(chunkExtent.getCount());

    // Save our tiles into the snapshot as chunks.
    unsigned int startLinearTileIndex{0};
    int chunksProcessed{0};
    for (unsigned int i = 0; i < chunkExtent.getCount(); ++i) {
        ChunkSnapshot& chunk{mapSnapshot.chunks[i]};

        // Process each tile in this chunk.
        unsigned int nextLinearTileIndex{startLinearTileIndex};
        unsigned int tilesProcessed{0};
        for (unsigned int j = 0; j < SharedConfig::CHUNK_TILE_COUNT; ++j) {
            // Copy all of the tile's layers into the snapshot.
            TileSnapshot& tile{chunk.tiles[j]};
            for (Tile::SpriteLayer& layer :
                 tiles[nextLinearTileIndex].spriteLayers) {
                const std::string& stringID{
                    spriteData.getStringID(layer.sprite.numericID)};
                unsigned int paletteID{chunk.getPaletteIndex(stringID)};
                tile.spriteLayers.push_back(paletteID);
            }

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
    Serialize::toFile((Paths::BASE_PATH + fileName), mapSnapshot);

    // Print the time taken.
    double timeTaken{timer.getDeltaSeconds(false)};
    LOG_INFO("Map saved in %.6fs.", timeTaken);
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
    for (unsigned int chunkIndex = 0; chunkIndex < mapSnapshot.chunks.size();
         ++chunkIndex) {
        // Calc the coordinates of this chunk's first tile.
        unsigned int startX{(chunkIndex % chunkExtent.xLength)
                            * SharedConfig::CHUNK_WIDTH};
        unsigned int startY{(chunkIndex / chunkExtent.xLength)
                            * SharedConfig::CHUNK_WIDTH};
        ChunkSnapshot& chunk{mapSnapshot.chunks[chunkIndex]};

        // These vars track which tile we're looking at, with respect to the
        // top left of the chunk.
        unsigned int relativeX{0};
        unsigned int relativeY{0};

        // Add all of this chunk's tiles to the tiles vector.
        for (unsigned int i = 0; i < SharedConfig::CHUNK_TILE_COUNT; ++i) {
            // Push all of the snapshot's sprites into the tile.
            TileSnapshot& tileSnapshot{chunk.tiles[i]};
            unsigned int layerIndex{0};
            for (unsigned int paletteID : tileSnapshot.spriteLayers) {
                const Sprite& sprite{spriteData.get(chunk.palette[paletteID])};
                setTileSpriteLayer((startX + relativeX), (startY + relativeY),
                                   layerIndex++, sprite);
            }

            // Increment the relative indices, wrapping at the chunk width.
            relativeX++;
            if (relativeX == SharedConfig::CHUNK_WIDTH) {
                relativeY++;
                relativeX = 0;
            }
        }
    }
}

} // End namespace Server
} // End namespace AM
