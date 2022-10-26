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
                                     std::size_t layerIndex,
                                     const Sprite& sprite)
{
    AM_ASSERT(tileX >= tileExtent.x, "x out of bounds: %d", tileX);
    AM_ASSERT(tileX <= tileExtent.xMax(), "x out of bounds: %d", tileX);
    AM_ASSERT(tileY >= tileExtent.y, "y out of bounds: %d", tileY);
    AM_ASSERT(tileY <= tileExtent.yMax(), "y out of bounds: %d", tileY);

    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    std::vector<Tile::SpriteLayer>& spriteLayers{tile.spriteLayers};

    // If the layer is already set to the given sprite, exit early.
    if ((spriteLayers.size() > layerIndex)
        && (spriteLayers[layerIndex].sprite.numericID == sprite.numericID)) {
        return;
    }

    // If we're being asked to set the highest layer in the tile to the empty 
    // sprite, erase it and any empties below it instead (to reduce space).
    if ((sprite.numericID == EMPTY_SPRITE_ID) 
        && (layerIndex == (spriteLayers.size() - 1))) {
        // Erase the sprite.
        spriteLayers.erase(spriteLayers.begin() + layerIndex);

        // Erase any empty sprites below it.
        for (std::size_t endIndex = (layerIndex - 1); endIndex-- > 0; ) {
            if (spriteLayers[endIndex].sprite.numericID == EMPTY_SPRITE_ID) {
                spriteLayers.erase(spriteLayers.begin() + endIndex);
            }
            else {
                break;
            }
        }
    }
    // Else, set the sprite layer.
    else {
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
        // Note: This sets intermediate layers to the empty sprite.
        if (spriteLayers.size() <= layerIndex) {
            spriteLayers.resize(
                (layerIndex + 1),
                {spriteData.get(EMPTY_SPRITE_ID), BoundingBox{}});
        }

        // Replace the sprite.
        spriteLayers[layerIndex] = {sprite, worldBounds};
    }

    // If we're tracking dirty tile state, update it.
    if (trackDirtyState) {
        // Set the lowest dirty layer index, unless there's already a lower
        // one being tracked.
        auto [iterator, didEmplace]
            = dirtyTiles.try_emplace({tileX, tileY}, layerIndex);
        if (!didEmplace && (layerIndex < iterator->second)) {
            iterator->second = layerIndex;
        }
    }
}

void TileMapBase::setTileSpriteLayer(int tileX, int tileY,
                                     std::size_t layerIndex,
                                     const std::string& stringID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(stringID));
}

void TileMapBase::setTileSpriteLayer(int tileX, int tileY,
                                     std::size_t layerIndex, int numericID)
{
    setTileSpriteLayer(tileX, tileY, layerIndex, spriteData.get(numericID));
}

bool TileMapBase::clearTile(int tileX, int tileY)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};

    bool layersWereCleared{false};
    if (tile.spriteLayers.size() > 0) {
        layersWereCleared = true;

        // If we're tracking dirty tile state, update it.
        if (trackDirtyState) {
            // Set the lowest dirty layer index.
            // Note: We don't have to check before setting, since 0 is lowest.
            dirtyTiles[{tileX, tileY}] = 0;
        }
    }

    tile.spriteLayers.clear();
    return layersWereCleared;
}

bool TileMapBase::clearTile(int tileX, int tileY,
    std::size_t startLayerIndex)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    std::vector<Tile::SpriteLayer>& spriteLayers{tile.spriteLayers};

    // If the start index is beyond this tile's highest layer, return false.
    if ((startLayerIndex + 1) > spriteLayers.size()) {
        return false;
    }

    // If we're tracking dirty tile state, update it.
    if (trackDirtyState) {
        // Set the lowest dirty layer index, unless there's already a lower
        // one being tracked.
        auto [iterator, didEmplace]
            = dirtyTiles.try_emplace({tileX, tileY}, startLayerIndex);
        if (!didEmplace && (startLayerIndex < iterator->second)) {
            iterator->second = startLayerIndex;
        }
    }

    // Erase the chosen layers and return true.
    spriteLayers.erase((spriteLayers.begin() + startLayerIndex),
                       spriteLayers.end());
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

bool TileMapBase::clearExtent(TileExtent extent, std::size_t startLayerIndex)
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

void TileMapBase::clear()
{
    chunkExtent = {};
    tileExtent = {};
    tiles.clear();
    dirtyTiles.clear();
}

const Tile& TileMapBase::getTile(int x, int y) const
{
    // TODO: How do we linearize negative coords?
    AM_ASSERT(x >= 0, "Negative coords not yet supported");
    AM_ASSERT(y >= 0, "Negative coords not yet supported");

    std::size_t tileIndex{linearizeTileIndex(x, y)};
    std::size_t maxTileIndex{
        static_cast<std::size_t>(tileExtent.xLength * tileExtent.yLength)};
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

std::unordered_map<TilePosition, std::size_t>& TileMapBase::getDirtyTiles()
{
    return dirtyTiles;
}

} // End namespace AM
