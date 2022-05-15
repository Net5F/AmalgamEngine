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
: chunkExtent{}
, tileExtent{}
, spriteData{inSpriteData}
{
    if (Config::RUN_OFFLINE) {
        LOG_INFO("Offline mode. Constructing default tile map.");

        // Set our map size.
        setMapSize(1, 1);

        // Fill every tile with a ground layer.
        const Sprite& ground{spriteData.get("test_6")};
        for (Tile& tile : tiles) {
            tile.spriteLayers.emplace_back(&ground, BoundingBox{});
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

void TileMap::setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
                                 unsigned int layerIndex, const Sprite& sprite)
{
    // If the sprite has a bounding box, calculate its position.
    BoundingBox worldBounds{};
    if (sprite.hasBoundingBox) {
        Position tilePosition{
            static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
            static_cast<float>(tileY * SharedConfig::TILE_WORLD_WIDTH), 0};
        worldBounds
            = Transforms::modelToWorld(sprite.modelBounds, tilePosition);
    }

    // If the tile's layers vector isn't big enough, resize it.
    // Note: This sets new layers to the "empty sprite".
    Tile& tile = tiles[linearizeTileIndex(tileX, tileY)];
    if (tile.spriteLayers.size() <= layerIndex) {
        const Sprite& emptySprite{spriteData.get(-1)};
        tile.spriteLayers.resize((layerIndex + 1),
                                 {&emptySprite, BoundingBox{}});
    }

    // Replace the sprite.
    tile.spriteLayers[layerIndex] = {&sprite, worldBounds};
}

void TileMap::setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
                                 unsigned int layerIndex,
                                 const std::string& stringID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(stringID));
}

void TileMap::setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
                                 unsigned int layerIndex, int numericID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(numericID));
}

void TileMap::clearTile(unsigned int tileX, unsigned int tileY)
{
    Tile& tile = tiles[linearizeTileIndex(tileX, tileY)];
    tile.spriteLayers.clear();
}

const Tile& TileMap::getTile(unsigned int x, unsigned int y) const
{
    unsigned int tileIndex{linearizeTileIndex(x, y)};
    unsigned int maxTileIndex{
        static_cast<unsigned int>(tileExtent.xLength * tileExtent.yLength)};
    AM_ASSERT((tileIndex < maxTileIndex),
              "Tried to get an out of bounds tile. tileIndex: %u, max: %u",
              tileIndex, maxTileIndex);
    ignore(maxTileIndex);

    return tiles[tileIndex];
}

const ChunkExtent& TileMap::getChunkExtent() const
{
    return chunkExtent;
}

const TileExtent& TileMap::getTileExtent() const
{
    return tileExtent;
}

} // End namespace Client
} // End namespace AM
