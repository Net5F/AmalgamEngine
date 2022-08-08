#include "TileMap.h"
#include "SpriteData.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "Config.h"
#include "SharedConfig.h"
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"
#include "Ignore.h"

namespace AM
{
namespace Client
{
TileMap::TileMap(SpriteData& inSpriteData)
: TileMapBase{inSpriteData}
{
    if (Config::RUN_OFFLINE) {
        LOG_INFO("Offline mode. Constructing default tile map.");

        // Set our map size.
        setMapSize(1, 1);

        // Fill every tile with a ground layer.
        const Sprite& ground{spriteData.get("test_6")};
        for (Tile& tile : tiles) {
            tile.spriteLayers.emplace_back(ground, BoundingBox{});
        }

        // Add some rugs to layer 1.
        const Sprite& rug{spriteData.get("test_15")};
        setTileSpriteLayer(0, 3, 1, rug);
        setTileSpriteLayer(4, 3, 1, rug);
        setTileSpriteLayer(3, 6, 1, rug);
        setTileSpriteLayer(2, 9, 1, rug);
        setTileSpriteLayer(1, 5, 1, rug);

        // Add some walls to layer 2.
        const Sprite& wall1{spriteData.get("test_17")};
        setTileSpriteLayer(2, 0, 2, wall1);
        setTileSpriteLayer(2, 1, 2, wall1);
        setTileSpriteLayer(2, 2, 2, wall1);

        const Sprite& wall2{spriteData.get("test_26")};
        setTileSpriteLayer(0, 2, 2, wall2);
    }
}

void TileMap::setMapSize(unsigned int inMapXLengthChunks,
                         unsigned int inMapYLengthChunks)
{
    // Set our map size.
    // Note: We set x/y to 0 since our map origin is always (0, 0). Change
    //       this if we ever support negative origins.
    chunkExtent.x = 0;
    chunkExtent.y = 0;
    chunkExtent.xLength = inMapXLengthChunks;
    chunkExtent.yLength = inMapYLengthChunks;
    tileExtent.x = 0;
    tileExtent.y = 0;
    tileExtent.xLength = (chunkExtent.xLength * SharedConfig::CHUNK_WIDTH);
    tileExtent.yLength = (chunkExtent.yLength * SharedConfig::CHUNK_WIDTH);

    // Resize the tiles vector to fit the map.
    tiles.resize(tileExtent.xLength * tileExtent.yLength);
}

} // End namespace Client
} // End namespace AM
