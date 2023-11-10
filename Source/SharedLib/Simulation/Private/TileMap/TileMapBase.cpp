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
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"
#include "Ignore.h"
#include <algorithm>

namespace AM
{
TileMapBase::TileMapBase(SpriteDataBase& inSpriteData, bool inTrackTileUpdates)
: spriteData{inSpriteData}
, chunkExtent{}
, tileExtent{}
, tiles{}
, trackTileUpdates{inTrackTileUpdates}
{
}

void TileMapBase::setFloor(int tileX, int tileY, const FloorSpriteSet& spriteSet)
{
    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    tile.getFloor().spriteSet = &spriteSet;

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(TileAddLayer{
            tileX, tileY, TileLayer::Type::Floor, spriteSet.numericID, 0});
    }
}

void TileMapBase::setFloor(int tileX, int tileY, const std::string& spriteSetID)
{
    setFloor(tileX, tileY, spriteData.getFloorSpriteSet(spriteSetID));
}

void TileMapBase::setFloor(int tileX, int tileY, Uint16 spriteSetID)
{
    setFloor(tileX, tileY, spriteData.getFloorSpriteSet(spriteSetID));
}

bool TileMapBase::remFloor(int tileX, int tileY)
{
    return clearTileLayers<FloorTileLayer>(tileX, tileY);
}

void TileMapBase::addFloorCovering(int tileX, int tileY, const FloorCoveringSpriteSet& spriteSet,
                      Rotation::Direction rotation)
{
    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    tile.getFloorCoverings().emplace_back(&spriteSet, rotation);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tileX, tileY, TileLayer::Type::FloorCovering,
                         spriteSet.numericID, static_cast<Uint8>(rotation)});
    }
}

void TileMapBase::addFloorCovering(int tileX, int tileY,
                                   const std::string& spriteSetID,
                                   Rotation::Direction rotation)
{
    addFloorCovering(tileX, tileY,
                     spriteData.getFloorCoveringSpriteSet(spriteSetID), rotation);
}

void TileMapBase::addFloorCovering(int tileX, int tileY, Uint16 spriteSetID,
                                   Rotation::Direction rotation)
{
    addFloorCovering(tileX, tileY,
                     spriteData.getFloorCoveringSpriteSet(spriteSetID), rotation);
}

bool TileMapBase::remFloorCovering(int tileX, int tileY,
                                   const FloorCoveringSpriteSet& spriteSet,
                                   Rotation::Direction direction)
{
    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};

    // Erase any layers with the same sprite set and rotation.
    std::size_t erasedCount{std::erase_if(
        tile.getFloorCoverings(),
        [&spriteSet, direction](const FloorCoveringTileLayer& layer) {
            if ((layer.spriteSet == &spriteSet)
                && (layer.direction == direction)) {
                return true;
            }
            return false;
        })};

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates && (erasedCount > 0)) {
        tileUpdateHistory.emplace_back(
            TileRemoveLayer{tileX, tileY, TileLayer::Type::FloorCovering,
                            spriteSet.numericID, direction});
    }

    return (erasedCount > 0);
}

bool TileMapBase::remFloorCovering(int tileX, int tileY,
                                   const std::string& spriteSetID,
                                   Rotation::Direction rotation)
{
    return remFloorCovering(tileX, tileY,
                     spriteData.getFloorCoveringSpriteSet(spriteSetID), rotation);
}

bool TileMapBase::remFloorCovering(int tileX, int tileY, Uint16 spriteSetID,
                                   Rotation::Direction rotation)
{
    return remFloorCovering(tileX, tileY,
                     spriteData.getFloorCoveringSpriteSet(spriteSetID), rotation);
}

void TileMapBase::addWall(int tileX, int tileY, const WallSpriteSet& spriteSet,
                      Wall::Type wallType)
{
    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    switch (wallType) {
        case Wall::Type::North: {
            addNorthWall(tileX, tileY, spriteSet);
            break;
        }
        case Wall::Type::West: {
            addWestWall(tileX, tileY, spriteSet);
            break;
        }
        default: {
            LOG_FATAL("Wall type must be North or West.");
            break;
        }
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tileX, tileY, TileLayer::Type::Wall,
                         spriteSet.numericID, static_cast<Uint8>(wallType)});
    }
}

void TileMapBase::addWall(int tileX, int tileY, const std::string& spriteSetID,
                          Wall::Type wallType)
{
    addWall(tileX, tileY, spriteData.getWallSpriteSet(spriteSetID), wallType);
}

void TileMapBase::addWall(int tileX, int tileY, Uint16 spriteSetID,
                          Wall::Type wallType)
{
    addWall(tileX, tileY, spriteData.getWallSpriteSet(spriteSetID), wallType);
}

bool TileMapBase::remWall(int tileX, int tileY, Wall::Type wallType)
{
    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    bool wallWasRemoved{false};
    switch (wallType) {
        case Wall::Type::North:
        {
            wallWasRemoved = remNorthWall(tileX, tileY);
            break;
        }
        case Wall::Type::West:
        {
            wallWasRemoved = remWestWall(tileX, tileY);
            break;
        }
        default: {
            LOG_FATAL("Wall type must be North or West.");
            break;
        }
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates && wallWasRemoved) {
        // Note: We don't care about sprite set ID when removing walls.
        tileUpdateHistory.emplace_back(
            TileRemoveLayer{tileX, tileY, TileLayer::Type::Wall,
                            0, static_cast<Uint8>(wallType)});
    }

    return wallWasRemoved;
}

void TileMapBase::addObject(int tileX, int tileY, const ObjectSpriteSet& spriteSet,
                      Rotation::Direction rotation)
{
    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    tile.getObjects().emplace_back(&spriteSet, rotation);

    // Rebuild the affected tile's collision.
    tile.rebuildCollision(tileX, tileY);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tileX, tileY, TileLayer::Type::Object,
                         spriteSet.numericID, static_cast<Uint8>(rotation)});
    }
}

void TileMapBase::addObject(int tileX, int tileY, const std::string& spriteSetID,
                            Rotation::Direction rotation)
{
    addObject(tileX, tileY, spriteData.getObjectSpriteSet(spriteSetID), rotation);
}

void TileMapBase::addObject(int tileX, int tileY, Uint16 spriteSetID,
                            Rotation::Direction rotation)
{
    addObject(tileX, tileY, spriteData.getObjectSpriteSet(spriteSetID), rotation);
}

bool TileMapBase::remObject(int tileX, int tileY,
                                   const ObjectSpriteSet& spriteSet,
                                   Rotation::Direction rotation)
{
    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};

    // Erase any layers with the same sprite set and rotation.
    std::size_t erasedCount{std::erase_if(
        tile.getObjects(),
        [&spriteSet, rotation](const ObjectTileLayer& layer) {
            if ((layer.spriteSet == &spriteSet)
                && (layer.direction == rotation)) {
                return true;
            }
            return false;
        })};

    // If an object was removed, rebuild the affected tile's collision.
    if (erasedCount > 0) {
        tile.rebuildCollision(tileX, tileY);
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates && (erasedCount > 0)) {
        tileUpdateHistory.emplace_back(
            TileRemoveLayer{tileX, tileY, TileLayer::Type::Object,
                            spriteSet.numericID, static_cast<Uint8>(rotation)});
    }

    return (erasedCount > 0);
}

bool TileMapBase::remObject(int tileX, int tileY, const std::string& spriteSetID,
                            Rotation::Direction rotation)
{
    return remObject(tileX, tileY, spriteData.getObjectSpriteSet(spriteSetID), rotation);
}

bool TileMapBase::remObject(int tileX, int tileY, Uint16 spriteSetID,
                            Rotation::Direction rotation)
{
    return remObject(tileX, tileY, spriteData.getObjectSpriteSet(spriteSetID), rotation);
}

bool TileMapBase::clearTileLayers(
    int tileX, int tileY,
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    bool layerWasCleared{
        clearTileLayersInternal(tileX, tileY, layerTypesToClear)};

    // If a layer was cleared, rebuild the affected tile's collision.
    if (layerWasCleared) {
        Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
        tile.rebuildCollision(tileX, tileY);
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(TileClearLayers{
            tileX, tileY, layerTypesToClear});
    }

    return layerWasCleared;
}

bool TileMapBase::clearTile(int tileX, int tileY)
{
    return clearTileLayers<FloorTileLayer, FloorCoveringTileLayer, WallTileLayer,
                    ObjectTileLayer>(tileX, tileY);
}

bool TileMapBase::clearExtentLayers(
    const TileExtent& extent,
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    AM_ASSERT(tileExtent.containsExtent(extent),
              "Tile extent out of bounds: %d, %d, %d, %d.", extent.x, extent.y,
              extent.xLength, extent.yLength);

    // Clear the given layers from each tile in the given extent.
    bool layerWasCleared{false};
    for (int x = extent.x; x <= extent.xMax(); ++x) {
        for (int y = extent.y; y <= extent.yMax(); ++y) {
            if (clearTileLayersInternal(x, y, layerTypesToClear)) {
                layerWasCleared = true;

                // A layer was cleared. Rebuild the affected tile's collision.
                Tile& tile{tiles[linearizeTileIndex(x, y)]};
                tile.rebuildCollision(x, y);
            }
        }
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(TileExtentClearLayers{
            extent, layerTypesToClear});
    }

    return layerWasCleared;
}

bool TileMapBase::clearExtent(const TileExtent& extent)
{
    return clearExtentLayers<FloorTileLayer, FloorCoveringTileLayer, WallTileLayer,
                      ObjectTileLayer>(extent);
}

void TileMapBase::clear()
{
    chunkExtent = {};
    tileExtent = {};
    tiles.clear();
    tileUpdateHistory.clear();
}

const Tile& TileMapBase::getTile(int tileX, int tileY) const
{
    // TODO: How do we linearize negative coords?
    AM_ASSERT(tileX >= 0, "Negative coords not yet supported");
    AM_ASSERT(tileY >= 0, "Negative coords not yet supported");

    AM_ASSERT(tileExtent.containsPosition({tileX, tileY}),
              "Tile coords out of bounds: %d, %d.", tileX, tileY);

    return tiles[linearizeTileIndex(tileX, tileY)];
}

const ChunkExtent& TileMapBase::getChunkExtent() const
{
    return chunkExtent;
}

const TileExtent& TileMapBase::getTileExtent() const
{
    return tileExtent;
}

const std::vector<TileMapBase::TileUpdateVariant>&
    TileMapBase::getTileUpdateHistory()
{
    return tileUpdateHistory;
}

void TileMapBase::clearTileUpdateHistory()
{
    tileUpdateHistory.clear();
}

void TileMapBase::addNorthWall(int tileX, int tileY, const WallSpriteSet& spriteSet)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    std::array<WallTileLayer, 2>& walls{tile.getWalls()};

    // If the tile has a West wall, add a NE gap fill.
    if (walls[0].wallType == Wall::Type::West) {
        walls[1].spriteSet = &spriteSet;
        walls[1].wallType = Wall::Type::NorthEastGapFill;
    }
    else {
        // No West wall, add the North wall.
        // Note: If there's a NorthWestGapFill, this will replace it.
        walls[1].spriteSet = &spriteSet;
        walls[1].wallType = Wall::Type::North;
    }

    // Rebuild the affected tile's collision.
    tile.rebuildCollision(tileX, tileY);

    // If there's a tile to the NE that we might've formed a corner with.
    if (tileExtent.containsPosition({tileX + 1, tileY - 1})) {
        Tile& northeastTile{tiles[linearizeTileIndex(tileX + 1, tileY - 1)]};
        std::array<WallTileLayer, 2>& northeastWalls{northeastTile.getWalls()};

        // If the NorthEast tile has a West wall.
        if (northeastWalls[0].wallType == Wall::Type::West) {
            // We formed a corner. Check if the tile to the East has a wall.
            // Note: We know this tile is valid cause there's a NorthEast tile.
            Tile& eastTile{tiles[linearizeTileIndex(tileX + 1, tileY)]};
            std::array<WallTileLayer, 2>& eastWalls{eastTile.getWalls()};
            if ((eastWalls[0].wallType == Wall::Type::None)
                && (eastWalls[1].wallType == Wall::Type::None)) {
                // The East tile has no walls. Add a NorthWestGapFill.
                eastWalls[1].spriteSet = &spriteSet;
                eastWalls[1].wallType = Wall::Type::NorthWestGapFill;
                eastTile.rebuildCollision(tileX + 1, tileY);
            }
            else if (eastWalls[1].wallType == Wall::Type::NorthWestGapFill) {
                // The East tile has a NW gap fill. If its sprite set no longer 
                // matches either surrounding wall, make it match the new wall.
                int gapFillID{eastWalls[1].spriteSet->numericID};
                int newNorthID{spriteSet.numericID};
                int westID{northeastWalls[0].spriteSet->numericID};
                if ((gapFillID != newNorthID) && (gapFillID != westID)) {
                    eastWalls[1].spriteSet = &spriteSet;
                }
            }
        }
    }
}

void TileMapBase::addWestWall(int tileX, int tileY, const WallSpriteSet& spriteSet)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    std::array<WallTileLayer, 2>& walls{tile.getWalls()};

    // Add the West wall.
    // Note: If this tile already has a West wall, this will replace it.
    walls[0].spriteSet = &spriteSet;
    walls[0].wallType = Wall::Type::West;

    // If the tile has a North wall, switch it to a NorthEast gap fill.
    if (walls[1].wallType == Wall::Type::North) {
        // Note: We don't change the sprite set. Only the type changes.
        walls[1].wallType = Wall::Type::NorthEastGapFill;
    }
    // Else if the tile has a NorthWest gap fill, remove it.
    else if (walls[1].wallType == Wall::Type::NorthWestGapFill) {
        walls[1].spriteSet = nullptr;
        walls[1].wallType = Wall::Type::None;
    }

    // Rebuild the affected tile's collision.
    tile.rebuildCollision(tileX, tileY);

    // If there's a tile to the SW that we might've formed a corner with.
    if (tileExtent.containsPosition({tileX - 1, tileY + 1})) {
        Tile& southwestTile{tiles[linearizeTileIndex(tileX - 1, tileY + 1)]};
        std::array<WallTileLayer, 2>& southwestWalls{southwestTile.getWalls()};

        // If the SouthWest tile has a North wall or a NE gap fill.
        if ((southwestWalls[1].wallType == Wall::Type::North) ||
            (southwestWalls[1].wallType == Wall::Type::NorthEastGapFill)) {
            // We formed a corner. Check if the tile to the South has a wall.
            // Note: We know this tile is valid cause there's a SouthWest tile.
            Tile& southTile{tiles[linearizeTileIndex(tileX, tileY + 1)]};
            std::array<WallTileLayer, 2>& southWalls{southTile.getWalls()};
            if ((southWalls[0].wallType == Wall::Type::None)
                && (southWalls[1].wallType == Wall::Type::None)) {
                // The South tile has no walls. Add a NorthWestGapFill.
                southWalls[1].spriteSet = &spriteSet;
                southWalls[1].wallType = Wall::Type::NorthWestGapFill;
                southTile.rebuildCollision(tileX, tileY + 1);
            }
            else if (southWalls[1].wallType == Wall::Type::NorthWestGapFill) {
                // The South tile has a NW gap fill. If its sprite set no longer 
                // matches either surrounding wall, make it match the new wall.
                int gapFillID{southWalls[1].spriteSet->numericID};
                int newWestID{spriteSet.numericID};
                int northID{southwestWalls[1].spriteSet->numericID};
                if ((gapFillID != newWestID) && (gapFillID != northID)) {
                    southWalls[1].spriteSet = &spriteSet;
                }
            }
        }
    }
}

bool TileMapBase::remNorthWall(int tileX, int tileY)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    std::array<WallTileLayer, 2>& walls{tile.getWalls()};

    bool wallWasRemoved{false};

    // If the tile has a North wall or NE gap fill, remove it.
    if ((walls[1].wallType == Wall::Type::North)
        || (walls[1].wallType == Wall::Type::NorthEastGapFill)) {
        walls[1].spriteSet = nullptr;
        walls[1].wallType = Wall::Type::None;
        tile.rebuildCollision(tileX, tileY);
        wallWasRemoved = true;
    }

    // If a wall was removed, check if there's a NW gap fill to the East.
    if (wallWasRemoved && (tileExtent.containsPosition({tileX + 1, tileY}))) {
        Tile& eastTile{tiles[linearizeTileIndex(tileX + 1, tileY)]};
        std::array<WallTileLayer, 2>& eastWalls{eastTile.getWalls()};
        if (eastWalls[1].wallType == Wall::Type::NorthWestGapFill) {
            // The East tile has a gap fill for a corner that we just broke.
            // Remove the gap fill.
            eastWalls[1].spriteSet = nullptr;
            eastWalls[1].wallType = Wall::Type::None;
            eastTile.rebuildCollision(tileX + 1, tileY);
        }
    }

    return wallWasRemoved;
}

bool TileMapBase::remWestWall(int tileX, int tileY)
{
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    std::array<WallTileLayer, 2>& walls{tile.getWalls()};

    bool wallWasRemoved{false};

    // If the tile has a West wall, remove it.
    if (walls[0].wallType == Wall::Type::West) {
        walls[0].spriteSet = nullptr;
        walls[0].wallType = Wall::Type::None;

        // If the tile has a NE gap fill, change it to a North.
        if (walls[1].wallType == Wall::Type::NorthEastGapFill) {
            walls[1].wallType = Wall::Type::North;
        }

        tile.rebuildCollision(tileX, tileY);
        wallWasRemoved = true;
    }

    // If a wall was removed, check if there's a NW gap fill to the South
    if (wallWasRemoved && (tileExtent.containsPosition({tileX, tileY + 1}))) {
        Tile& southTile{tiles[linearizeTileIndex(tileX, tileY + 1)]};
        std::array<WallTileLayer, 2>& southWalls{southTile.getWalls()};
        if (southWalls[1].wallType == Wall::Type::NorthWestGapFill) {
            // The South tile has a gap fill for a corner that we just broke.
            // Remove the gap fill.
            southWalls[1].spriteSet = nullptr;
            southWalls[1].wallType = Wall::Type::None;
            southTile.rebuildCollision(tileX, tileY + 1);
        }
    }

    return wallWasRemoved;
}

bool TileMapBase::clearTileLayersInternal(
    int tileX, int tileY,
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    bool layerWasCleared{false};
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    if (layerTypesToClear[TileLayer::Type::Floor]) {
        if (tile.getFloor().spriteSet != nullptr) {
            tile.getFloor().spriteSet = nullptr;
            layerWasCleared = true;
        }
    }

    if (layerTypesToClear[TileLayer::Type::FloorCovering]) {
        if (tile.getFloorCoverings().size() != 0) {
            tile.getFloorCoverings().clear();
            layerWasCleared = true;
        }
    }

    if (layerTypesToClear[TileLayer::Type::Wall]) {
        std::array<WallTileLayer, 2>& walls{tile.getWalls()};
        if ((walls[0].wallType != Wall::Type::None)
            || (walls[1].wallType != Wall::Type::None)) {
            walls[0].spriteSet = nullptr;
            walls[0].wallType = Wall::Type::None;
            walls[1].spriteSet = nullptr;
            walls[1].wallType = Wall::Type::None;
            layerWasCleared = true;
        }
    }

    if (layerTypesToClear[TileLayer::Type::Object]) {
        if (tile.getObjects().size() != 0) {
            tile.getObjects().clear();
            layerWasCleared = true;
        }
    }

    return layerWasCleared;
}

} // End namespace AM
