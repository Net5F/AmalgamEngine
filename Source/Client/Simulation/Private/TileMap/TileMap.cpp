#include "TileMap.h"
#include "SpriteData.h"
#include "Position.h"
#include "MovementHelpers.h"

namespace AM
{
namespace Client
{

TileMap::TileMap(SpriteData& inSpriteData)
: spriteData{inSpriteData}
{
    // TODO: Load this from some storage format.
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
}

void TileMap::addSpriteLayer(unsigned int tileX, unsigned int tileY, const Sprite& sprite)
{
    // If the sprite has a bounding box, calculate its position.
    BoundingBox fixedBounds{};
    if (sprite.hasBoundingBox) {
        Position tilePosition{
            static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
            static_cast<float>(tileY * SharedConfig::TILE_WORLD_HEIGHT), 0};
        MovementHelpers::moveBoundingBox(tilePosition, fixedBounds);
    }

    // Push the sprite into the tile's layers vector.
    unsigned int linearizedIndex
        = tileY * SharedConfig::WORLD_WIDTH + tileX;
    Tile& tile = tiles[linearizedIndex];
    tile.spriteLayers.emplace_back(&sprite, fixedBounds);
}

void TileMap::replaceSpriteLayer(unsigned int tileX, unsigned int tileY
                                 , unsigned int layerIndex, const Sprite& sprite)
{
    // If the sprite has a bounding box, calculate its position.
    BoundingBox fixedBounds{};
    if (sprite.hasBoundingBox) {
        Position tilePosition{
            static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
            static_cast<float>(tileY * SharedConfig::TILE_WORLD_HEIGHT), 0};
        MovementHelpers::moveBoundingBox(tilePosition, fixedBounds);
    }

    // Replace the sprite.
    unsigned int linearizedIndex
        = tileY * SharedConfig::WORLD_WIDTH + tileX;
    Tile& tile = tiles[linearizedIndex];
    tile.spriteLayers[layerIndex] = {&sprite, fixedBounds};
}

const Tile& TileMap::get(int x, int y) const
{
    unsigned int linearizedIndex
        = y * SharedConfig::WORLD_WIDTH + x;
    return tiles[linearizedIndex];
}

std::size_t TileMap::size()
{
    return TILE_COUNT;
}

std::size_t TileMap::sizeX()
{
    return SharedConfig::WORLD_WIDTH;
}

std::size_t TileMap::sizeY()
{
    return SharedConfig::WORLD_HEIGHT;
}

} // End namespace Client
} // End namespace AM
