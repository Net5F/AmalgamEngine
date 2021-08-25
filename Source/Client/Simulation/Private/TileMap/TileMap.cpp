#include "TileMap.h"
#include "SpriteData.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "ByteTools.h"
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

    // Open the map file.
    std::string fullPath{Paths::BASE_PATH};
    fullPath += "World.map";
    std::ifstream mapFile(fullPath, std::ios::binary);

    // Load the map data into memory.
    std::vector<Uint8> mapData;
    if (mapFile.is_open()) {
        // Determine the size of the file.
        mapFile.seekg(0, std::ios::end);
        std::streampos fileSize{mapFile.tellg()};
        mapData.resize(fileSize);

        // Load the file's data into the vector.
        mapFile.seekg(0, std::ios::beg);
        mapFile.read(reinterpret_cast<char*>(&mapData[0]), fileSize);
    }
    else {
        LOG_ERROR("World.map failed to open.");
    }

    // Parse the map file.
    parseMap(mapData);
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

unsigned long int TileMap::getTileCount() const
{
    return tileCount;
}

void TileMap::parseMap(const std::vector<Uint8>& mapData)
{
    // Read the version number.
    unsigned long bufferIndex{0};
    unsigned int versionNumber{ByteTools::read16(&mapData[bufferIndex])};
    ignore(versionNumber);
    bufferIndex += 2;

    // Read the X/Y mapLengths.
    mapXLengthChunks = ByteTools::read32(&mapData[bufferIndex]);
    mapXLengthTiles = mapXLengthChunks * SharedConfig::CHUNK_WIDTH;
    bufferIndex += 4;
    mapYLengthChunks = ByteTools::read32(&mapData[bufferIndex]);
    mapYLengthTiles = mapYLengthChunks * SharedConfig::CHUNK_WIDTH;
    bufferIndex += 4;

    // Allocate space for our tiles.
    tileCount = mapXLengthTiles * mapYLengthTiles;
    tiles.resize(tileCount);

    /* Parse the chunks. */
    // For each row of chunks.
    for (unsigned int y = 0; y < mapYLengthChunks; ++y) {
        // For each chunk in this row.
        for (unsigned int x = 0; x < mapXLengthChunks; ++x) {
            // Parse the chunk.
            parseChunk(mapData, bufferIndex, x, y);
        }
    }
}

void TileMap::parseChunk(const std::vector<Uint8>& mapData, unsigned long& bufferIndex
                         , unsigned int chunkX, unsigned int chunkY)
{
    /* Get all of the palette's string IDs. */
    std::vector<std::string> palette;
    while (mapData[bufferIndex] != '\0') {
        // Determine how long the string is.
        unsigned long endIndex{bufferIndex};
        while (mapData[endIndex] != '\0') {
            endIndex++;
            if (endIndex == 100) {
                LOG_ERROR("Searched 100 bytes before giving up looking for "
                "end of palette string.");
            }
        }

        // Push the string into the palette vector.
        palette.emplace_back(&mapData[bufferIndex], &mapData[endIndex]);
        bufferIndex += (endIndex - bufferIndex);

        // Increment to the start of the next string.
        bufferIndex++;
    }

    // Increment past the ending '\0'.
    bufferIndex++;

    // Add all the chunk's tiles
    unsigned int chunkStartX = (chunkX * SharedConfig::CHUNK_WIDTH);
    unsigned int chunkStartY = (chunkY * SharedConfig::CHUNK_WIDTH);
    unsigned int chunkEndX = chunkStartX + SharedConfig::CHUNK_WIDTH;
    unsigned int chunkEndY = chunkStartY + SharedConfig::CHUNK_WIDTH;
    for (unsigned int tileY = chunkStartY; tileY < chunkEndY; ++tileY) {
        for (unsigned int tileX = chunkStartX; tileX < chunkEndX; ++tileX) {
            // Get the number of layers in this tile.
            unsigned int numLayers{mapData[bufferIndex]};
            bufferIndex++;

            // Add the layers.
            for (unsigned int i = 0; i < numLayers; ++i) {
                unsigned int paletteIndex{mapData[bufferIndex]};
                bufferIndex++;

                if (paletteIndex != 0) {
                    for (unsigned int j = 0; j < 20; ++j) {
                        std::printf("%u ", mapData[bufferIndex + j]);
                    }
                    std::printf("\n");
                    std::fflush(stdout);
                }

                addSpriteLayer(tileX, tileY, spriteData.get(palette[paletteIndex]));
            }
        }
    }
}

void TileMap::save(const std::string& fileName)
{
    std::vector<Uint8> mapData;
    unsigned long bufferIndex{0};

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
