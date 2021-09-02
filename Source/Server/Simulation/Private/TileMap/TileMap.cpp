#include "TileMap.h"
#include "SpriteData.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "SharedConfig.h"
#include "Timer.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Server
{
TileMap::TileMap(SpriteData& inSpriteData)
: mapXLengthChunks{0}
, mapYLengthChunks{0}
, mapXLengthTiles{0}
, mapYLengthTiles{0}
, tileCount{0}
, spriteData{inSpriteData}
{
    // Prime a timer.
    Timer timer;
    timer.updateSavedTime();

    // Deserialize the file into a snapshot.
    TileMapSnapshot mapSnapshot;
    Deserialize::fromFile((Paths::BASE_PATH + "TileMap.bin"), mapSnapshot);

    // Load the map snapshot.
    loadMap(mapSnapshot);

    // Print the time taken.
    double timeTaken = timer.getDeltaSeconds(false);
    LOG_INFO("Map loaded in %.6fs.", timeTaken);
}

void TileMap::addSpriteLayer(unsigned int tileX, unsigned int tileY,
                             const Sprite& sprite)
{
    // If the sprite has a bounding box, calculate its position.
    BoundingBox worldBounds{};
    if (sprite.hasBoundingBox) {
        Position tilePosition{
            static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
            static_cast<float>(tileY * SharedConfig::TILE_WORLD_HEIGHT), 0};
        worldBounds
            = Transforms::modelToWorld(sprite.modelBounds, tilePosition);
    }

    // Push the sprite into the tile's layers vector.
    unsigned int linearizedIndex = (tileY * mapXLengthTiles) + tileX;
    Tile& tile = tiles[linearizedIndex];
    tile.spriteLayers.emplace_back(&sprite, worldBounds);
}

void TileMap::replaceSpriteLayer(unsigned int tileX, unsigned int tileY,
                                 unsigned int layerIndex, const Sprite& sprite)
{
    // If the sprite has a bounding box, calculate its position.
    BoundingBox worldBounds{};
    if (sprite.hasBoundingBox) {
        Position tilePosition{
            static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
            static_cast<float>(tileY * SharedConfig::TILE_WORLD_HEIGHT), 0};
        worldBounds
            = Transforms::modelToWorld(sprite.modelBounds, tilePosition);
    }

    // Replace the sprite.
    unsigned int linearizedIndex = (tileY * mapXLengthTiles) + tileX;
    Tile& tile = tiles[linearizedIndex];
    tile.spriteLayers[layerIndex] = {&sprite, worldBounds};
}

const Tile& TileMap::get(unsigned int x, unsigned int y) const
{
    unsigned int linearizedIndex = (y * mapXLengthTiles) + x;
    return tiles[linearizedIndex];
}

unsigned int TileMap::xLengthChunks() const
{
    return mapXLengthChunks;
}

unsigned int TileMap::yLengthChunks() const
{
    return mapYLengthChunks;
}

unsigned int TileMap::xLengthTiles() const
{
    return mapXLengthTiles;
}

unsigned int TileMap::yLengthTiles() const
{
    return mapYLengthTiles;
}

unsigned int TileMap::getTileCount() const
{
    return tileCount;
}

void TileMap::loadMap(TileMapSnapshot& mapSnapshot)
{
    /* Load the snapshot into this map. */
    // Load the header data.
    mapXLengthChunks = mapSnapshot.xLengthChunks;
    mapYLengthChunks = mapSnapshot.yLengthChunks;
    mapXLengthTiles = mapXLengthChunks * SharedConfig::CHUNK_WIDTH;
    mapYLengthTiles = mapYLengthChunks * SharedConfig::CHUNK_WIDTH;

    // Resize the tiles vector to fit the map.
    tiles.resize(mapXLengthTiles * mapYLengthTiles);

    // Load the chunks into the tiles vector.
    for (unsigned int chunkIndex = 0; chunkIndex < mapSnapshot.chunks.size(); ++chunkIndex) {
        // Calc the coordinates of this chunk's first tile.
        unsigned int startX{(chunkIndex % mapXLengthChunks) * SharedConfig::CHUNK_WIDTH};
        unsigned int startY{(chunkIndex / mapXLengthChunks) * SharedConfig::CHUNK_WIDTH};
        ChunkSnapshot& chunk{mapSnapshot.chunks[chunkIndex]};

        // These vars track which tile we're looking at, with respect to the
        // top left of the chunk.
        unsigned int relativeX{0};
        unsigned int relativeY{0};

        // Add all of this chunk's tiles to the tiles vector.
        for (unsigned int i = 0; i < SharedConfig::CHUNK_TILE_COUNT; ++i) {
            // Push all of the snapshot's sprites into the tile.
            TileSnapshot& tileSnapshot{chunk.tiles[i]};
            for (unsigned int paletteId : tileSnapshot.spriteLayers) {
                const Sprite& sprite{spriteData.get(chunk.palette[paletteId])};
                addSpriteLayer((startX + relativeX), (startY + relativeY), sprite);
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

void TileMap::save(const std::string& fileName)
{
    // Prime a timer.
    Timer timer;
    timer.updateSavedTime();

    /* Save this map's state into a snapshot. */
    // Save the header data.
    TileMapSnapshot mapSnapshot{};
    mapSnapshot.version = MAP_FORMAT_VERSION;
    mapSnapshot.xLengthChunks = mapXLengthChunks;
    mapSnapshot.yLengthChunks = mapYLengthChunks;

    // Allocate room for our chunks.
    unsigned int chunkCount{mapXLengthChunks * mapYLengthChunks};
    mapSnapshot.chunks.resize(chunkCount);

    // Save our tiles into the snapshot as chunks.
    unsigned int startIndex{0};
    unsigned int chunksProcessed{0};
    for (unsigned int i = 0; i < chunkCount; ++i) {
        ChunkSnapshot& chunk{mapSnapshot.chunks[i]};

        // Process each tile in this chunk.
        unsigned int nextTileIndex{startIndex};
        unsigned int tilesProcessed{0};
        for (unsigned int j = 0; j < SharedConfig::CHUNK_TILE_COUNT; ++j) {
            // Copy all of this tile's layers.
            TileSnapshot& tile{chunk.tiles[j]};
            for (Tile::SpriteLayer& layer : tiles[nextTileIndex].spriteLayers) {
                unsigned int paletteId{chunk.getPaletteIndex(layer.sprite->stringId)};
                tile.spriteLayers.push_back(paletteId);
            }

            // Increment to the next tile.
            nextTileIndex++;

            // If we've processed all the tiles in this row, increment to the
            // next row.
            tilesProcessed++;
            if (tilesProcessed == SharedConfig::CHUNK_WIDTH) {
                nextTileIndex += (mapXLengthTiles - SharedConfig::CHUNK_WIDTH);
                tilesProcessed = 0;
            }
        }

        // Increment to the next chunk.
        startIndex += SharedConfig::CHUNK_WIDTH;

        // If we've processed all the chunks in this row, increment to the
        // next row.
        chunksProcessed++;
        if (chunksProcessed == mapXLengthChunks) {
            startIndex += ((SharedConfig::CHUNK_WIDTH - 1) * mapXLengthTiles);
            chunksProcessed = 0;
        }
    }

    // Serialize the map snapshot and write it to the file.
    Serialize::toFile((Paths::BASE_PATH + fileName), mapSnapshot);

    // Print the time taken.
    double timeTaken = timer.getDeltaSeconds(false);
    LOG_INFO("Map saved in %.6fs.", timeTaken);
}

} // End namespace Server
} // End namespace AM
