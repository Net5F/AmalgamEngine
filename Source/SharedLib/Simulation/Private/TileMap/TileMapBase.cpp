#include "TileMapBase.h"
#include "GraphicDataBase.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "Morton.h"
#include "SharedConfig.h"
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
TileMapBase::TileMapBase(GraphicDataBase& inGraphicData, bool inTrackTileUpdates)
: graphicData{inGraphicData}
, chunkExtent{}
, tileExtent{}
, chunks{}
, autoRebuildCollision{true}
, dirtyCollisionQueue{}
, trackTileUpdates{inTrackTileUpdates}
, tileUpdateHistory{}
{
}

void TileMapBase::setFloor(const TilePosition& tilePosition,
                           const FloorGraphicSet& graphicSet)
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    // If there's an existing floor, replace it.
    Tile& tile{getTile(tilePosition)};
    if (TileLayer* floor{tile.findLayer(TileLayer::Type::Floor)}) {
        floor->graphicSet = graphicSet;
    }
    else {
        // No existing floor, add one.
        tile.addLayer(TileLayer::Type::Floor, graphicSet, 0);
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(tile, tilePosition);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tilePosition, TileLayer::Type::Floor,
                         static_cast<Uint16>(graphicSet.numericID), 0});
    }
}

void TileMapBase::setFloor(const TilePosition& tilePosition,
                           const std::string& graphicSetID)
{
    setFloor(tilePosition, graphicData.getFloorGraphicSet(graphicSetID));
}

void TileMapBase::setFloor(const TilePosition& tilePosition,
                           Uint16 graphicSetID)
{
    setFloor(tilePosition, graphicData.getFloorGraphicSet(graphicSetID));
}

bool TileMapBase::remFloor(const TilePosition& tilePosition)
{
    // Since there's only 1 floor per tile, we can just clear it.
    return clearTileLayers(tilePosition, {TileLayer::Type::Floor});
}

void TileMapBase::addFloorCovering(const TilePosition& tilePosition,
                                   const FloorCoveringGraphicSet& graphicSet,
                                   Rotation::Direction rotation)
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    Tile& tile{getTile(tilePosition)};
    tile.addLayer(TileLayer::Type::FloorCovering, graphicSet, rotation);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tilePosition, TileLayer::Type::FloorCovering,
                         static_cast<Uint16>(graphicSet.numericID),
                         static_cast<Uint8>(rotation)});
    }
}

void TileMapBase::addFloorCovering(const TilePosition& tilePosition,
                                   const std::string& graphicSetID,
                                   Rotation::Direction rotation)
{
    addFloorCovering(tilePosition,
                     graphicData.getFloorCoveringGraphicSet(graphicSetID),
                     rotation);
}

void TileMapBase::addFloorCovering(const TilePosition& tilePosition, Uint16 graphicSetID,
                                   Rotation::Direction rotation)
{
    addFloorCovering(tilePosition,
                     graphicData.getFloorCoveringGraphicSet(graphicSetID),
                     rotation);
}

bool TileMapBase::remFloorCovering(const TilePosition& tilePosition,
                                   const FloorCoveringGraphicSet& graphicSet,
                                   Rotation::Direction rotation)
{
    return remFloorCovering(tilePosition, graphicSet.numericID, rotation);
}

bool TileMapBase::remFloorCovering(const TilePosition& tilePosition,
                                   const std::string& graphicSetID,
                                   Rotation::Direction rotation)
{
    return remFloorCovering(
        tilePosition,
        graphicData.getFloorCoveringGraphicSet(graphicSetID).numericID,
        rotation);
}

bool TileMapBase::remFloorCovering(const TilePosition& tilePosition, Uint16 graphicSetID,
                                   Rotation::Direction rotation)
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    // Remove any matching layers.
    Tile& tile{getTile(tilePosition)};
    bool layerWasRemoved{tile.removeLayer(TileLayer::Type::FloorCovering,
                                          graphicSetID, rotation)};

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates && layerWasRemoved) {
        tileUpdateHistory.emplace_back(
            TileRemoveLayer{tilePosition, TileLayer::Type::FloorCovering,
                            graphicSetID, rotation});
    }

    return layerWasRemoved;
}

void TileMapBase::addWall(const TilePosition& tilePosition, const WallGraphicSet& graphicSet,
                          Wall::Type wallType)
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    switch (wallType) {
        case Wall::Type::North: {
            addNorthWall(tilePosition, graphicSet);
            break;
        }
        case Wall::Type::West: {
            addWestWall(tilePosition, graphicSet);
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
            TileAddLayer{tilePosition, TileLayer::Type::Wall,
                         static_cast<Uint16>(graphicSet.numericID),
                         static_cast<Uint8>(wallType)});
    }
}

void TileMapBase::addWall(const TilePosition& tilePosition, const std::string& graphicSetID,
                          Wall::Type wallType)
{
    addWall(tilePosition, graphicData.getWallGraphicSet(graphicSetID), wallType);
}

void TileMapBase::addWall(const TilePosition& tilePosition, Uint16 graphicSetID,
                          Wall::Type wallType)
{
    addWall(tilePosition, graphicData.getWallGraphicSet(graphicSetID), wallType);
}

bool TileMapBase::remWall(const TilePosition& tilePosition, Wall::Type wallType)
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    bool wallWasRemoved{false};
    switch (wallType) {
        case Wall::Type::North: {
            wallWasRemoved = remNorthWall(tilePosition);
            break;
        }
        case Wall::Type::West: {
            wallWasRemoved = remWestWall(tilePosition);
            break;
        }
        default: {
            LOG_FATAL("Wall type must be North or West.");
            break;
        }
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates && wallWasRemoved) {
        // Note: We don't care about graphic set ID when removing walls.
        tileUpdateHistory.emplace_back(
            TileRemoveLayer{tilePosition, TileLayer::Type::Wall, 0,
                            static_cast<Uint8>(wallType)});
    }

    return wallWasRemoved;
}

void TileMapBase::addObject(const TilePosition& tilePosition,
                            const ObjectGraphicSet& graphicSet,
                            Rotation::Direction rotation)
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    Tile& tile{getTile(tilePosition)};
    tile.addLayer(TileLayer::Type::Object, graphicSet, rotation);

    // Rebuild the affected tile's collision.
    rebuildTileCollision(tile, tilePosition);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tilePosition, TileLayer::Type::Object,
                         static_cast<Uint16>(graphicSet.numericID),
                         static_cast<Uint8>(rotation)});
    }
}

void TileMapBase::addObject(const TilePosition& tilePosition,
                            const std::string& graphicSetID,
                            Rotation::Direction rotation)
{
    addObject(tilePosition, graphicData.getObjectGraphicSet(graphicSetID),
              rotation);
}

void TileMapBase::addObject(const TilePosition& tilePosition, Uint16 graphicSetID,
                            Rotation::Direction rotation)
{
    addObject(tilePosition, graphicData.getObjectGraphicSet(graphicSetID),
              rotation);
}

bool TileMapBase::remObject(const TilePosition& tilePosition,
                            const ObjectGraphicSet& graphicSet,
                            Rotation::Direction rotation)
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    // Remove any matching layers.
    Tile& tile{getTile(tilePosition)};
    bool layerWasRemoved{tile.removeLayer(TileLayer::Type::Object,
                                          graphicSet.numericID, rotation)};

    // If an object was removed, rebuild the affected tile's collision.
    if (layerWasRemoved) {
        rebuildTileCollision(tile, tilePosition);
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates && layerWasRemoved) {
        tileUpdateHistory.emplace_back(
            TileRemoveLayer{tilePosition, TileLayer::Type::Object,
                            static_cast<Uint16>(graphicSet.numericID),
                            static_cast<Uint8>(rotation)});
    }

    return layerWasRemoved;
}

bool TileMapBase::remObject(const TilePosition& tilePosition,
                            const std::string& graphicSetID,
                            Rotation::Direction rotation)
{
    return remObject(tilePosition, graphicData.getObjectGraphicSet(graphicSetID),
                     rotation);
}

bool TileMapBase::remObject(const TilePosition& tilePosition, Uint16 graphicSetID,
                            Rotation::Direction rotation)
{
    return remObject(tilePosition, graphicData.getObjectGraphicSet(graphicSetID),
                     rotation);
}

bool TileMapBase::clearTileLayers(
    const TilePosition& tilePosition,
    const std::initializer_list<TileLayer::Type>& layerTypesToClear)
{
    return clearTileLayers(tilePosition,
                           toBoolArray(layerTypesToClear));
}

bool TileMapBase::clearTileLayers(
    const TilePosition& tilePosition,
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    bool layerWasCleared{
        clearTileLayersInternal(tilePosition, layerTypesToClear)};

    // If a layer was cleared, rebuild the affected tile's collision.
    if (layerWasCleared) {
        Tile& tile{getTile(tilePosition)};
        rebuildTileCollision(tile, tilePosition);
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileClearLayers{tilePosition, layerTypesToClear});
    }

    return layerWasCleared;
}

bool TileMapBase::clearTile(const TilePosition& tilePosition)
{
    return clearTileLayers(tilePosition,
                           {TileLayer::Type::Floor,
                            TileLayer::Type::FloorCovering,
                            TileLayer::Type::Wall, TileLayer::Type::Object});
}

bool TileMapBase::clearExtentLayers(const TileExtent& extent,
    const std::initializer_list<TileLayer::Type>& layerTypesToClear)
{
    return clearExtentLayers(extent, toBoolArray(layerTypesToClear));
}

bool TileMapBase::clearExtentLayers(
    const TileExtent& extent,
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    AM_ASSERT(tileExtent.containsExtent(extent),
              "Tile extent out of bounds: %d, %d, %d, %d, %d, %d.", extent.x,
              extent.y, extent.z, extent.xLength, extent.yLength,
              extent.zLength);

    // Clear the given layers from each tile in the given extent.
    bool layerWasCleared{false};
    for (int z{extent.z}; z <= extent.zMax(); ++z) {
        for (int y{extent.y}; y <= extent.yMax(); ++y) {
            for (int x{extent.x}; x <= extent.xMax(); ++x) {
                TilePosition tilePosition{x, y, z};
                if (clearTileLayersInternal(tilePosition, layerTypesToClear)) {
                    layerWasCleared = true;

                    // A layer was cleared. Rebuild the affected tile's
                    // collision.
                    Tile& tile{getTile(tilePosition)};
                    rebuildTileCollision(tile, tilePosition);
                }
            }
        }
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileExtentClearLayers{extent, layerTypesToClear});
    }

    return layerWasCleared;
}

bool TileMapBase::clearExtent(const TileExtent& extent)
{
    return clearExtentLayers(
        extent, {TileLayer::Type::Floor, TileLayer::Type::FloorCovering,
                 TileLayer::Type::Wall, TileLayer::Type::Object});
}

void TileMapBase::clear()
{
    chunkExtent = {};
    tileExtent = {};
    chunks.clear();
    tileUpdateHistory.clear();
}

const Chunk& TileMapBase::getChunk(const ChunkPosition& chunkPosition) const
{
    std::size_t chunkIndex{linearizeChunkIndex(chunkPosition)};
    if (chunkIndex >= chunks.size()) {
        LOG_ERROR("Invalid chunk requested: (%d, %d, %d)", chunkPosition.x,
                  chunkPosition.y, chunkPosition.z);
        return chunks[0];
    }

    return chunks[chunkIndex];
}

const Tile& TileMapBase::getTile(const TilePosition& tilePosition) const
{
    AM_ASSERT(tileExtent.containsPosition(tilePosition),
              "Tile coords out of bounds: %d, %d, %d.", tilePosition.x,
              tilePosition.y, tilePosition.z);

    // Get the index of the chunk containing the tile.
    ChunkPosition chunkPosition{tilePosition};
    std::size_t chunkIndex{linearizeChunkIndex(chunkPosition)};

    // Get the tile's relative position within the chunk.
    const int CHUNK_WIDTH{static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    int relativeTileX{tilePosition.x - (chunkPosition.x * CHUNK_WIDTH)};
    int relativeTileY{tilePosition.y - (chunkPosition.y * CHUNK_WIDTH)};

    return chunks[chunkIndex].getTile(relativeTileX, relativeTileY);
}

const Tile& TileMapBase::cgetTile(const TilePosition& tilePosition) const
{
    return getTile(tilePosition);
}

const ChunkExtent& TileMapBase::getChunkExtent() const
{
    return chunkExtent;
}

const TileExtent& TileMapBase::getTileExtent() const
{
    return tileExtent;
}

void TileMapBase::setAutoRebuildCollision(bool newAutoRebuildCollision)
{
    // If we're re-enabling auto rebuild, rebuild any dirty tiles.
    if (!autoRebuildCollision && newAutoRebuildCollision) {
        rebuildDirtyTileCollision();
    }

    autoRebuildCollision = newAutoRebuildCollision;
}

void TileMapBase::rebuildDirtyTileCollision()
{
    // De-duplicate the queue.
    std::sort(dirtyCollisionQueue.begin(), dirtyCollisionQueue.end());
    auto lastIt{
        std::unique(dirtyCollisionQueue.begin(), dirtyCollisionQueue.end())};

    // Rebuild the collision of any dirty tiles.
    for (auto it{dirtyCollisionQueue.begin()}; it != lastIt; ++it) {
        Tile& tile{getTile(*it)};
        tile.rebuildCollision(*it);
    }

    dirtyCollisionQueue.clear();
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

Tile& TileMapBase::getTile(const TilePosition& tilePosition)
{
    return const_cast<Tile&>(
        static_cast<const TileMapBase&>(*this).getTile(tilePosition));
}

void TileMapBase::rebuildTileCollision(Tile& tile, const TilePosition& tilePosition)
{
    // If auto rebuild is enabled, rebuild the affected tile's collision.
    if (autoRebuildCollision) {
        tile.rebuildCollision(tilePosition);
    }
    else {
        // Not enabled. Queue the affected tile to have its collision rebuilt.
        dirtyCollisionQueue.emplace_back(tilePosition);
    }
}

void TileMapBase::addNorthWall(const TilePosition& tilePosition,
                               const WallGraphicSet& graphicSet)
{
    Tile& tile{getTile(tilePosition)};

    // If the tile has a West wall, add a NE gap fill.
    if (tile.findLayer(TileLayer::Type::Wall, Wall::Type::West)) {
        tile.addLayer(TileLayer::Type::Wall, graphicSet,
                      Wall::Type::NorthEastGapFill);
    }
    else {
        // No West wall. If there's an existing North wall or NW gap fill, 
        // replace it.
        bool replacedWall{false};
        for (TileLayer& layer : tile.getLayers(TileLayer::Type::Wall)) {
            if ((layer.graphicIndex == Wall::Type::North)
                || (layer.graphicIndex == Wall::Type::NorthWestGapFill)) {
                layer.graphicSet = graphicSet;
                layer.graphicIndex = Wall::Type::North;
                replacedWall = true;
                break;
            }
        }

        // If there was no existing North wall, add one.
        if (!replacedWall) {
            tile.addLayer(TileLayer::Type::Wall, graphicSet, Wall::Type::North);
        }
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(tile, tilePosition);

    // If there's a tile to the NE that we might've formed a corner with.
    if (tileExtent.containsPosition(
            {tilePosition.x + 1, tilePosition.y - 1, tilePosition.z})) {
        Tile& northeastTile{
            getTile({tilePosition.x + 1, tilePosition.y - 1, tilePosition.z})};

        // If the NorthEast tile has a West wall.
        if (TileLayer* 
              northeastWestWall{northeastTile.findLayer(TileLayer::Type::Wall,
                                                        Wall::Type::West)}) {
            // We formed a corner. Check if the tile to the East has a wall.
            // Note: We know this tile is valid cause there's a NorthEast tile.
            Tile& eastTile{
                getTile({tilePosition.x + 1, tilePosition.y, tilePosition.z})};
            if (eastTile.getLayers(TileLayer::Type::Wall).size() == 0) {
                // The East tile has no walls. Add a NorthWestGapFill.
                eastTile.addLayer(TileLayer::Type::Wall, graphicSet,
                                  Wall::Type::NorthWestGapFill);
                rebuildTileCollision(
                    eastTile,
                    {tilePosition.x + 1, tilePosition.y, tilePosition.z});
            }
            else if (TileLayer* eastNorthWestGapFill{
                         eastTile.findLayer(TileLayer::Type::Wall,
                                            Wall::Type::NorthWestGapFill)}) {
                // The East tile has a NW gap fill. If its graphic set no longer
                // matches either surrounding wall, make it match the new wall.
                int gapFillID{eastNorthWestGapFill->graphicSet.get().numericID};
                int newNorthID{graphicSet.numericID};
                int westID{northeastWestWall->graphicSet.get().numericID};
                if ((gapFillID != newNorthID) && (gapFillID != westID)) {
                    eastNorthWestGapFill->graphicSet = graphicSet;
                }
                rebuildTileCollision(
                    eastTile,
                    {tilePosition.x + 1, tilePosition.y, tilePosition.z});
            }
        }
    }
}

void TileMapBase::addWestWall(const TilePosition& tilePosition,
                              const WallGraphicSet& graphicSet)
{
    Tile& tile{getTile(tilePosition)};

    // If there's an existing West wall, replace it.
    if (TileLayer* westWall{tile.findLayer(TileLayer::Type::Wall,
                                           Wall::Type::West)}) {
        westWall->graphicSet = graphicSet;
    }
    else {
        // No existing West wall, add one.
        tile.addLayer(TileLayer::Type::Wall, graphicSet, Wall::Type::West);
    }

    // If the tile has a North wall, switch it to a NorthEast gap fill.
    if (TileLayer* northWall{tile.findLayer(TileLayer::Type::Wall,
                                            Wall::Type::North)}) {
        // Note: We don't change the graphic set. Only the type changes.
        northWall->graphicIndex = Wall::Type::NorthEastGapFill;
    }
    // Else if the tile has a NorthWest gap fill, remove it.
    else {
        tile.removeLayers(TileLayer::Type::Wall, Wall::Type::NorthWestGapFill);
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(tile, tilePosition);

    // If there's a tile to the SW that we might've formed a corner with.
    if (tileExtent.containsPosition(
            {tilePosition.x - 1, tilePosition.y + 1, tilePosition.z})) {
        Tile& southwestTile{
            getTile({tilePosition.x - 1, tilePosition.y + 1, tilePosition.z})};

        // If the SouthWest tile has a North wall or a NE gap fill.
        TileLayer* southwestNorthWall{
            southwestTile.findLayer(TileLayer::Type::Wall, Wall::Type::North)};
        TileLayer* southwestNorthEastGapFill{southwestTile.findLayer(
            TileLayer::Type::Wall, Wall::Type::NorthEastGapFill)};
        if (southwestNorthWall || southwestNorthEastGapFill) {
            // We formed a corner. Check if the tile to the South has a wall.
            // Note: We know this tile is valid cause there's a SouthWest tile.
            Tile& southTile{
                getTile({tilePosition.x, tilePosition.y + 1, tilePosition.z})};
            if (southTile.getLayers(TileLayer::Type::Wall).size() == 0) {
                // The South tile has no walls. Add a NorthWestGapFill.
                southTile.addLayer(TileLayer::Type::Wall, graphicSet,
                                   Wall::Type::NorthWestGapFill);
                rebuildTileCollision(
                    southTile,
                    {tilePosition.x, tilePosition.y + 1, tilePosition.z});
            }
            else if (TileLayer* southNorthWestGapFill{southTile.findLayer(
                         TileLayer::Type::Wall, Wall::Type::NorthWestGapFill)}) {
                // The South tile has a NW gap fill. If its graphic set no longer
                // matches either surrounding wall, make it match the new wall.
                int gapFillID{southNorthWestGapFill->graphicSet.get().numericID};
                int newWestID{graphicSet.numericID};
                int northID{southwestNorthWall
                                ? southwestNorthWall->graphicSet.get().numericID
                                : southwestNorthEastGapFill->graphicSet.get()
                                      .numericID};
                if ((gapFillID != newWestID) && (gapFillID != northID)) {
                    southNorthWestGapFill->graphicSet = graphicSet;
                }
                rebuildTileCollision(
                    southTile,
                    {tilePosition.x, tilePosition.y + 1, tilePosition.z});
            }
        }
    }
}

bool TileMapBase::remNorthWall(const TilePosition& tilePosition)
{
    Tile& tile{getTile(tilePosition)};

    // If the tile has a North wall or NE gap fill, remove it.
    bool wallWasRemoved{
        tile.removeLayers(TileLayer::Type::Wall, Wall::Type::North)};
    if (!wallWasRemoved) {
        wallWasRemoved = tile.removeLayers(
            TileLayer::Type::Wall, Wall::Type::NorthEastGapFill);
    }

    // If a wall was removed.
    if (wallWasRemoved) {
        // Rebuild the affected tile's collision.
        rebuildTileCollision(tile, tilePosition);

        // Check if there's a NW gap fill to the East.
        if (tileExtent.containsPosition(
                {tilePosition.x + 1, tilePosition.y, tilePosition.z})) {
            Tile& eastTile{
                getTile({tilePosition.x + 1, tilePosition.y, tilePosition.z})};
            // If the East tile has a gap fill for a corner that we just broke,
            // remove it.
            if (eastTile.removeLayers(TileLayer::Type::Wall,
                                      Wall::Type::NorthWestGapFill)) {
                rebuildTileCollision(
                    eastTile,
                    {tilePosition.x + 1, tilePosition.y, tilePosition.z});
            }
        }
    }

    return wallWasRemoved;
}

bool TileMapBase::remWestWall(const TilePosition& tilePosition)
{
    Tile& tile{getTile(tilePosition)};

    // If the tile has a West wall, remove it.
    if (tile.removeLayers(TileLayer::Type::Wall, Wall::Type::West)) {
        // If the tile has a NE gap fill, change it to a North.
        if (TileLayer* northEastGapFill{tile.findLayer(TileLayer::Type::Wall,
               Wall::Type::NorthEastGapFill)}) {
            northEastGapFill->graphicIndex = Wall::Type::North;
        }

        // Rebuild the affected tile's collision.
        rebuildTileCollision(tile, tilePosition);

        // Check if there's a NW gap fill to the South
        if (tileExtent.containsPosition(
                {tilePosition.x, tilePosition.y + 1, tilePosition.z})) {
            Tile& southTile{
                getTile({tilePosition.x, tilePosition.y + 1, tilePosition.z})};
            // If the South tile has a gap fill for a corner that we just broke,
            // remove it.
            if (southTile.removeLayers(TileLayer::Type::Wall,
                                       Wall::Type::NorthWestGapFill)) {
                dirtyCollisionQueue.emplace_back(
                    tilePosition.x, tilePosition.y + 1, tilePosition.z);
            }
        }

        return true;
    }

    return false;
}

bool TileMapBase::clearTileLayersInternal(
    const TilePosition& tilePosition,
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    bool layerWasCleared{false};
    Tile& tile{getTile(tilePosition)};

    // If we're being asked to clear every layer, clear the whole tile.
    if (layerTypesToClear[TileLayer::Type::Floor]
        && layerTypesToClear[TileLayer::Type::FloorCovering]
        && layerTypesToClear[TileLayer::Type::Wall]
        && layerTypesToClear[TileLayer::Type::Object]) {
        return tile.clear();
    }

    return tile.clearLayers(layerTypesToClear);
}

std::array<bool, TileLayer::Type::Count> TileMapBase::toBoolArray(
    const std::initializer_list<TileLayer::Type>& layerTypesToClear)
{
    std::array<bool, TileLayer::Type::Count> boolArray{};
    for (TileLayer::Type type : layerTypesToClear) {
        AM_ASSERT(type < TileLayer::Type::Count, "Invalid tile layer type.");
        boolArray[type] = true;
    }

    return boolArray;
}

} // End namespace AM
