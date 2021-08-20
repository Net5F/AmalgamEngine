#include "MapGenerator.h"
#include "ByteTools.h"
#include "Paths.h"
#include <iostream>
#include <cstring>
#include <fstream>

namespace AM
{
namespace MG
{

MapGenerator::MapGenerator(unsigned int inMapXLength
                           , unsigned int inMapYLength, const std::string& inFillSpriteId)
: mapXLength{inMapXLength}
, mapYLength{inMapYLength}
, fillSpriteId{inFillSpriteId}
, bufferIndex{0}
{
    /* Allocate the map data buffer's required size. */
    // Version + xlen + ylen.
    unsigned int headerSize{2 + 4 + 4};

    // Chunks per row * number of rows.
    unsigned int palletCount{(mapXLength / CHUNK_WIDTH) * (mapYLength / CHUNK_WIDTH)};
    // ID chars + '\0', + another '\0' to mark the end.
    unsigned int palletSize{static_cast<unsigned int>(fillSpriteId.length()) + 1 + 1};

    // Length of tiles in X * length of tiles in Y.
    unsigned int tileCount{mapXLength * mapYLength};
    // 1B number of sprites + 1B palette index.
    unsigned int tileSize{1 + 1};

    // Allocate the map data buffer.
    unsigned int requiredSize = headerSize + (palletCount * palletSize)
      + (tileCount * tileSize);
    mapData.resize(requiredSize);

    std::printf("Allocated memory for map: %u bytes\n", requiredSize);
    std::fflush(stdout);
}

void MapGenerator::generate()
{
    // Clear the map before starting.
    mapData.clear();
    bufferIndex = 0;

    // Add the version number.
    ByteTools::write16(MAP_FORMAT_VERSION, &mapData[bufferIndex]);
    bufferIndex += 2;

    // Add the lengths.
    ByteTools::write32(mapXLength, &mapData[bufferIndex]);
    bufferIndex += 4;
    ByteTools::write32(mapYLength, &mapData[bufferIndex]);
    bufferIndex += 4;

    /* Add the chunks. */
    // For each row of chunks.
    for (unsigned int y = 0; y < (mapYLength / CHUNK_WIDTH); ++y) {
        // For each chunk in this row.
        for (unsigned int x = 0; x < (mapXLength / CHUNK_WIDTH); ++x) {
            // Add the fill sprite to the palette.
            std::memcpy(&mapData[bufferIndex], fillSpriteId.c_str()
                        , (fillSpriteId.length() + 1));
            bufferIndex += (fillSpriteId.length() + 1);

            // End the palette with '\0'.
            mapData[bufferIndex] = '\0';
            bufferIndex++;

            // Add the tiles.
            for (unsigned int i = 0; i < CHUNK_TILE_COUNT; ++i) {
                // Add the number of layers in this tile.
                mapData[bufferIndex] = 1;
                bufferIndex++;

                // Add the palette index of the sprite in layer 0.
                mapData[bufferIndex] = 0;
                bufferIndex++;
            }
        }
    }
}

void MapGenerator::save(const std::string& fileName)
{
    // Create the file.
    std::ofstream workingFile(Paths::BASE_PATH + fileName);

    // Write our buffer contents to the file.
    workingFile.write(reinterpret_cast<char*>(mapData.data()), bufferIndex);
}

} // End namespace MG
} // End namespace AM
