#include "MapGenerator.h"
#include "ByteTools.h"
#include "Paths.h"
#include "SharedConfig.h"
#include "TileMapSnapshot.h"
#include "Serialize.h"
#include <iostream>
#include <cstring>
#include <fstream>

namespace AM
{
namespace MG
{
MapGenerator::MapGenerator(unsigned int inMapLengthX, unsigned int inMapLengthY,
                           const std::string& inFillSpriteId)
: mapXLength{inMapLengthX}
, mapYLength{inMapLengthY}
, fillSpriteId{inFillSpriteId}
, dataSize{0}
{
}

void MapGenerator::generateAndSave(const std::string& fileName)
{
    // Clear the map before starting.
    mapData.clear();

    // Fill the map's version and size.
    TileMapSnapshot tileMap;
    tileMap.version = MAP_FORMAT_VERSION;
    tileMap.xLengthChunks = mapXLength;
    tileMap.yLengthChunks = mapYLength;

    // Fill the chunks.
    tileMap.chunks.resize(mapXLength * mapYLength);
    for (ChunkSnapshot& chunk : tileMap.chunks) {
        // Push the sprite ID that we're filling the map with into the palette.
        chunk.palette.push_back(fillSpriteId);

        // Push the palette index of the sprite into each tile.
        for (unsigned int i = 0; i < 256; ++i) {
            chunk.tiles[i].spriteLayers.push_back(0);
        }
    }

    // Serialize the map snapshot and write it to the file.
    Serialize::toFile((Paths::BASE_PATH + fileName), tileMap);
}

} // End namespace MG
} // End namespace AM
