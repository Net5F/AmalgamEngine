#include "TileMapBase.h"
#include "SpriteDataBase.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "SharedConfig.h"
#include "EmptySpriteID.h"
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"
#include "Ignore.h"

namespace AM
{
TileMapBase::TileMapBase(SpriteDataBase& inSpriteData)
: spriteData{inSpriteData}
, chunkExtent{}
, tileExtent{}
, tiles{}
{
}

void TileMapBase::setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
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
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    if (tile.spriteLayers.size() <= layerIndex) {
        const Sprite& emptySprite{spriteData.get(EMPTY_SPRITE_ID)};
        tile.spriteLayers.resize((layerIndex + 1),
                                 {emptySprite, BoundingBox{}});
    }

    // Replace the sprite.
    tile.spriteLayers[layerIndex] = {sprite, worldBounds};
}

void TileMapBase::setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
                                 unsigned int layerIndex,
                                 const std::string& stringID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(stringID));
}

void TileMapBase::setTileSpriteLayer(unsigned int tileX, unsigned int tileY,
                                 unsigned int layerIndex, int numericID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(numericID));
}

void TileMapBase::clearTile(unsigned int tileX, unsigned int tileY)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    tile.spriteLayers.clear();
}

const Tile& TileMapBase::getTile(unsigned int x, unsigned int y) const
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

const ChunkExtent& TileMapBase::getChunkExtent() const
{
    return chunkExtent;
}

const TileExtent& TileMapBase::getTileExtent() const
{
    return tileExtent;
}

} // End namespace AM
