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
#include <algorithm>

namespace AM
{
TileMapBase::TileMapBase(SpriteDataBase& inSpriteData, bool inTrackDirtyState)
: spriteData{inSpriteData}
, chunkExtent{}
, tileExtent{}
, tiles{}
, trackDirtyState{inTrackDirtyState}
{
}

void TileMapBase::setTileSpriteLayer(int tileX, int tileY,
                                     unsigned int layerIndex,
                                     const Sprite& sprite)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    auto& spriteLayers{tile.spriteLayers};

    // If the layer is already set to the given sprite, exit early.
    if ((spriteLayers.size() > layerIndex)
        && (spriteLayers[layerIndex].sprite.numericID == sprite.numericID)) {
        return;
    }

    // Else, set the sprite layer.
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
    spriteLayers[layerIndex] = {sprite, worldBounds};

    // If we're tracking dirty state, add the updated tile's coordinates.
    if (trackDirtyState) {
        dirtyTiles.emplace(tileX, tileY);
    }
}

void TileMapBase::setTileSpriteLayer(int tileX, int tileY,
                                     unsigned int layerIndex,
                                     const std::string& stringID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(stringID));
}

void TileMapBase::setTileSpriteLayer(int tileX, int tileY,
                                     unsigned int layerIndex, int numericID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(numericID));
}

bool TileMapBase::clearTile(int tileX, int tileY)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};

    bool layersWereCleared{false};
    if (tile.spriteLayers.size() > 0) {
        layersWereCleared = true;

        // If we're tracking dirty state, add the updated tile's coordinates.
        if (trackDirtyState) {
            dirtyTiles.emplace(tileX, tileY);
        }
    }

    tile.spriteLayers.fill({spriteData.get(EMPTY_SPRITE_ID), {}});
    return layersWereCleared;
}

bool TileMapBase::clearTile(int tileX, int tileY,
    unsigned int startLayerIndex)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    auto& spriteLayers{tile.spriteLayers};

    // If the start index is beyond this tile's highest layer, return false.
    if ((startLayerIndex + 1) > spriteLayers.size()) {
        return false;
    }

    // Clear the chosen layers and return true.
    bool layerErased{false};
    for (auto it = (spriteLayers.begin() + startLayerIndex);
         it != spriteLayers.end(); ++it) {
        if (it->sprite.numericID != EMPTY_SPRITE_ID) {
            it->sprite = spriteData.get(EMPTY_SPRITE_ID);
            layerErased = true;
        }
    }

    // If we're tracking dirty state and any layers were cleared, add the 
    // updated tile's coordinates.
    if (trackDirtyState && layerErased) {
        dirtyTiles.emplace(tileX, tileY);
    }

    return true;
}

bool TileMapBase::clearExtent(TileExtent extent)
{
    bool layersWereCleared{false};

    // Clear every tile in the given extent.
    int xMax{extent.x + extent.xLength};
    int yMax{extent.y + extent.yLength};
    for (int x = extent.x; x < xMax; ++x) {
        for (int y = extent.y; y < yMax; ++y) {
            if (clearTile(x, y)) {
                layersWereCleared = true;
            }
        }
    }

    return layersWereCleared;
}

bool TileMapBase::clearExtent(TileExtent extent, unsigned int startLayerIndex)
{
    bool layersWereCleared{false};

    // Clear every layer above startLayerIndex in the given extent.
    int xMax{extent.x + extent.xLength};
    int yMax{extent.y + extent.yLength};
    for (int x = extent.x; x < xMax; ++x) {
        for (int y = extent.y; y < yMax; ++y) {
            if (clearTile(x, y, startLayerIndex)) {
                layersWereCleared = true;
            }
        }
    }

    return layersWereCleared;
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

std::unordered_set<TilePosition>& TileMapBase::getDirtyTiles()
{
    return dirtyTiles;
}

} // End namespace AM
