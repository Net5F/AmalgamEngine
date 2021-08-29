#include "TileMap.h"
#include "SpriteData.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "SharedConfig.h"
#include "Timer.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Client
{
TileMap::TileMap(SpriteData& inSpriteData)
: mapXLengthChunks{0}
, mapYLengthChunks{0}
, mapXLengthTiles{0}
, mapYLengthTiles{0}
, tileCount{0}
, spriteData{inSpriteData}
{
//    // Fill every tile with a ground layer.
//    const Sprite& ground{spriteData.get("test_6")};
//    for (Tile& tile : tiles) {
//        tile.spriteLayers.emplace_back(&ground, BoundingBox{});
//    }
//
//    // Add some rugs to layer 1.
//    const Sprite& rug{spriteData.get("test_15")};
//    addSpriteLayer(0, 3, rug);
//    addSpriteLayer(4, 3, rug);
//    addSpriteLayer(3, 6, rug);
//    addSpriteLayer(2, 9, rug);
//    addSpriteLayer(1, 5, rug);
//
//    // Add some walls to layer 2.
//    const Sprite& wall1{spriteData.get("test_17")};
//    addSpriteLayer(2, 0, wall1);
//    addSpriteLayer(2, 1, wall1);
//    addSpriteLayer(2, 2, wall1);
//
//    const Sprite& wall2{spriteData.get("test_26")};
//    addSpriteLayer(0, 2, wall2);
    // Prime a timer.
    Timer timer;
    timer.updateSavedTime();

    // Open the map file.
    std::string fullPath{Paths::BASE_PATH};
    fullPath += "TileMap.bin";
    std::ifstream mapFile(fullPath, std::ios::binary);

    // Load the map data into memory.
    BinaryBuffer mapData;
    if (mapFile.is_open()) {
        // Determine the size of the file.
        mapFile.seekg(0, std::ios::end);
        unsigned int fileSize{static_cast<unsigned int>(mapFile.tellg())};
        mapData.resize(fileSize);
        LOG_INFO("Allocated memory for map: %u bytes.", fileSize);

        // Load the file's data into the buffer.
        mapFile.seekg(0, std::ios::beg);
        mapFile.read(reinterpret_cast<char*>(&mapData[0]), fileSize);

        // Close the file.
        mapFile.close();
    }
    else {
        LOG_ERROR("TileMap.bin failed to open.");
    }

    // Deserialize the data into a snapshot.
    TileMapSnapshot mapSnapshot;
    Deserialize::fromBuffer(mapData, mapData.size(), mapSnapshot);

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

const Tile& TileMap::get(int x, int y) const
{
    unsigned int linearizedIndex = (y * mapYLengthTiles) + x;
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
    /* Load the data into this map. */
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
    std::vector<Uint8> mapData;
    unsigned bufferIndex{0};

    // Add the version number.
    ByteTools::write16(MAP_FORMAT_VERSION, &mapData[bufferIndex]);
    bufferIndex += 2;

    // Add the lengths.
    ByteTools::write32(mapXLengthChunks, &mapData[bufferIndex]);
    bufferIndex += 4;
    ByteTools::write32(mapYLengthChunks, &mapData[bufferIndex]);
    bufferIndex += 4;


    // Open or create the file.
    std::ofstream workingFile((Paths::BASE_PATH + fileName), std::ios::binary);

    // Write our buffer contents to the file.
//    workingFile.write(reinterpret_cast<char*>(mapData.data()), bufferIndex);
}

} // End namespace Client
} // End namespace AM
