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
//    if (Config::RUN_OFFLINE) {
//        LOG_INFO("Offline mode. Constructing default tile map.");

        // Set our map size.
        mapXLengthChunks = 1;
        mapYLengthChunks = 1;
        mapXLengthTiles = mapXLengthChunks * SharedConfig::CHUNK_WIDTH;
        mapYLengthTiles = mapYLengthChunks * SharedConfig::CHUNK_WIDTH;

        // Resize the tiles vector to fit the map.
        tiles.resize(mapXLengthTiles * mapYLengthTiles);

        // Fill every tile with a ground layer.
        const Sprite& ground{spriteData.get("test_6")};
        for (Tile& tile : tiles) {
            tile.spriteLayers.emplace_back(&ground, BoundingBox{});
        }

        // Add some rugs to layer 1.
        const Sprite& rug{spriteData.get("test_15")};
        addSpriteLayer(0, 3, rug);
        addSpriteLayer(4, 3, rug);
        addSpriteLayer(3, 6, rug);
        addSpriteLayer(2, 9, rug);
        addSpriteLayer(1, 5, rug);

        // Add some walls to layer 2.
        const Sprite& wall1{spriteData.get("test_17")};
        addSpriteLayer(2, 0, wall1);
        addSpriteLayer(2, 1, wall1);
        addSpriteLayer(2, 2, wall1);

        const Sprite& wall2{spriteData.get("test_26")};
        addSpriteLayer(0, 2, wall2);
//    }
}

void TileMap::addSpriteLayer(unsigned int tileX, unsigned int tileY,
                             const Sprite& sprite)
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

    // Push the sprite into the tile's layers vector.
    Tile& tile = tiles[linearizeTileIndex(tileX, tileY)];
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
            static_cast<float>(tileY * SharedConfig::TILE_WORLD_WIDTH), 0};
        worldBounds
            = Transforms::modelToWorld(sprite.modelBounds, tilePosition);
    }

    // Replace the sprite.
    Tile& tile = tiles[linearizeTileIndex(tileX, tileY)];
    tile.spriteLayers[layerIndex] = {&sprite, worldBounds};
}

const Tile& TileMap::getTile(unsigned int x, unsigned int y) const
{
    return tiles[linearizeTileIndex(x, y)];
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

} // End namespace Client
} // End namespace AM
