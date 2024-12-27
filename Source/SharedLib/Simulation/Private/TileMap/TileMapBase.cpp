#include "TileMapBase.h"
#include "GraphicDataBase.h"
#include "CollisionLocator.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "ChunkWireSnapshot.h"
#include "Morton.h"
#include "SharedConfig.h"
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
static_assert(SharedConfig::TILE_WORLD_WIDTH <= SDL_MAX_UINT8,
              "TILE_WORLD_WIDTH must fit within a Uint8 for TileOffset.");
static_assert(SharedConfig::TILE_WORLD_HEIGHT <= SDL_MAX_UINT8,
              "TILE_WORLD_HEIGHT must fit within a Uint8 for TileOffset.");

TileMapBase::TileMapBase(GraphicDataBase& inGraphicData,
                         CollisionLocator& inCollisionLocator,
                         bool inTrackTileUpdates)
: graphicData{inGraphicData}
, collisionLocator{inCollisionLocator}
, chunkExtent{}
, tileExtent{}
, chunks{}
, autoRebuildCollision{true}
, dirtyCollisionQueue{}
, trackTileUpdates{inTrackTileUpdates}
, tileUpdateHistory{}
{
}

void TileMapBase::addTerrain(const TilePosition& tilePosition,
                             const TerrainGraphicSet& graphicSet,
                             Terrain::Value terrainValue)
{
    // If the terrain height + start height is too tall to fit in the tile, 
    // return without adding.
    auto [terrainHeight, terrainStartHeight]{Terrain::getInfo(terrainValue)};
    if ((terrainHeight + terrainStartHeight) >= Terrain::MAX_COMBINED_HEIGHT) {
        return;
    }

    Tile* tile{addTileLayer(tilePosition, {}, TileLayer::Type::Terrain,
                            graphicSet, terrainValue)};
    if (!tile) {
        // tilePosition is outside of the map bounds.
        return;
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(*tile, tilePosition);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tilePosition,
                         {},
                         TileLayer::Type::Terrain,
                         static_cast<Uint16>(graphicSet.numericID),
                         static_cast<Uint8>(terrainValue)});
    }
}

void TileMapBase::addTerrain(const TilePosition& tilePosition,
                             const std::string& graphicSetID,
                             Terrain::Value terrainValue)
{
    addTerrain(tilePosition, graphicData.getTerrainGraphicSet(graphicSetID),
               terrainValue);
}

void TileMapBase::addTerrain(const TilePosition& tilePosition,
                             Uint16 graphicSetID, Terrain::Value terrainValue)
{
    addTerrain(tilePosition, graphicData.getTerrainGraphicSet(graphicSetID),
               terrainValue);
}

bool TileMapBase::remTerrain(const TilePosition& tilePosition)
{
    // Since there's only 1 terrain per tile, we can just clear it.
    return clearTileLayers(tilePosition, {TileLayer::Type::Terrain});
}

void TileMapBase::addFloor(const TilePosition& tilePosition,
                           const TileOffset& tileOffset,
                           const FloorGraphicSet& graphicSet,
                           Rotation::Direction rotation)
{
    Tile* tile{addTileLayer(tilePosition, tileOffset, TileLayer::Type::Floor,
                            graphicSet, rotation)};
    if (!tile) {
        // tilePosition is outside of the map bounds.
        return;
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(*tile, tilePosition);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tilePosition, tileOffset, TileLayer::Type::Floor,
                         static_cast<Uint16>(graphicSet.numericID),
                         static_cast<Uint8>(rotation)});
    }
}

void TileMapBase::addFloor(const TilePosition& tilePosition,
                           const TileOffset& tileOffset,
                           const std::string& graphicSetID,
                           Rotation::Direction rotation)
{
    addFloor(tilePosition, tileOffset,
             graphicData.getFloorGraphicSet(graphicSetID), rotation);
}

void TileMapBase::addFloor(const TilePosition& tilePosition,
                           const TileOffset& tileOffset, Uint16 graphicSetID,
                           Rotation::Direction rotation)
{
    addFloor(tilePosition, tileOffset,
             graphicData.getFloorGraphicSet(graphicSetID), rotation);
}

bool TileMapBase::remFloor(const TilePosition& tilePosition,
                           const TileOffset& tileOffset,
                           const FloorGraphicSet& graphicSet,
                           Rotation::Direction rotation)
{
    return remFloor(tilePosition, tileOffset, graphicSet.numericID, rotation);
}

bool TileMapBase::remFloor(const TilePosition& tilePosition,
                           const TileOffset& tileOffset,
                           const std::string& graphicSetID,
                           Rotation::Direction rotation)
{
    return remFloor(tilePosition, tileOffset,
                    graphicData.getFloorGraphicSet(graphicSetID).numericID,
                    rotation);
}

bool TileMapBase::remFloor(const TilePosition& tilePosition,
                           const TileOffset& tileOffset, Uint16 graphicSetID,
                           Rotation::Direction rotation)
{
    Tile* tile{remTileLayer(tilePosition, tileOffset, TileLayer::Type::Floor,
                            graphicSetID, rotation)};
    if (!tile) {
        // Floor wasn't found.
        return false;
    }

    // If a floor was removed, rebuild the affected tile's collision.
    rebuildTileCollision(*tile, tilePosition);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileRemoveLayer{tilePosition,
                            tileOffset,
                            TileLayer::Type::Floor,
                            graphicSetID,
                            static_cast<Uint8>(rotation)});
    }

    return true;
}

void TileMapBase::addWall(const TilePosition& tilePosition, const WallGraphicSet& graphicSet,
                          Wall::Type wallType)
{
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
            TileAddLayer{tilePosition,
                         {},
                         TileLayer::Type::Wall,
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
            TileRemoveLayer{tilePosition,
                            {},
                            TileLayer::Type::Wall,
                            0,
                            static_cast<Uint8>(wallType)});
    }

    return wallWasRemoved;
}

void TileMapBase::addObject(const TilePosition& tilePosition,
                            const TileOffset& tileOffset,
                            const ObjectGraphicSet& graphicSet,
                            Rotation::Direction rotation)
{
    Tile* tile{addTileLayer(tilePosition, tileOffset, TileLayer::Type::Object,
                            graphicSet, rotation)};
    if (!tile) {
        // tilePosition is outside of the map bounds.
        return;
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(*tile, tilePosition);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileAddLayer{tilePosition, tileOffset, TileLayer::Type::Object,
                         static_cast<Uint16>(graphicSet.numericID),
                         static_cast<Uint8>(rotation)});
    }
}

void TileMapBase::addObject(const TilePosition& tilePosition,
                            const TileOffset& tileOffset,
                            const std::string& graphicSetID,
                            Rotation::Direction rotation)
{
    addObject(tilePosition, tileOffset,
              graphicData.getObjectGraphicSet(graphicSetID), rotation);
}

void TileMapBase::addObject(const TilePosition& tilePosition,
                            const TileOffset& tileOffset, Uint16 graphicSetID,
                            Rotation::Direction rotation)
{
    addObject(tilePosition, tileOffset,
              graphicData.getObjectGraphicSet(graphicSetID), rotation);
}

bool TileMapBase::remObject(const TilePosition& tilePosition,
                            const TileOffset& tileOffset,
                            const ObjectGraphicSet& graphicSet,
                            Rotation::Direction rotation)
{
    return remObject(tilePosition, tileOffset, graphicSet.numericID, rotation);
}

bool TileMapBase::remObject(const TilePosition& tilePosition,
                            const TileOffset& tileOffset,
                            const std::string& graphicSetID,
                            Rotation::Direction rotation)
{
    return remObject(tilePosition, tileOffset,
                     graphicData.getObjectGraphicSet(graphicSetID).numericID,
                     rotation);
}

bool TileMapBase::remObject(const TilePosition& tilePosition,
                            const TileOffset& tileOffset, Uint16 graphicSetID,
                            Rotation::Direction rotation)
{
    Tile* tile{remTileLayer(tilePosition, tileOffset, TileLayer::Type::Object,
                            graphicSetID, rotation)};
    if (!tile) {
        // Object wasn't found.
        return false;
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(*tile, tilePosition);

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileRemoveLayer{tilePosition, tileOffset, TileLayer::Type::Object,
                            graphicSetID, static_cast<Uint8>(rotation)});
    }

    return true;
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
    Tile* clearedTile{
        clearTileLayersInternal(tilePosition, layerTypesToClear)};

    // If a layer was cleared, rebuild the affected tile's collision.
    if (clearedTile) {
        rebuildTileCollision(*clearedTile, tilePosition);
    }

    // If we're tracking tile updates, add this one to the history.
    if (trackTileUpdates) {
        tileUpdateHistory.emplace_back(
            TileClearLayers{tilePosition, layerTypesToClear});
    }

    return (clearedTile != nullptr);
}

bool TileMapBase::clearTile(const TilePosition& tilePosition)
{
    return clearTileLayers(tilePosition,
                           {TileLayer::Type::Terrain, TileLayer::Type::Floor,
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
    // Clear the given layers from each tile in the given extent.
    bool layerWasCleared{false};
    for (int z{extent.z}; z <= extent.zMax(); ++z) {
        for (int y{extent.y}; y <= extent.yMax(); ++y) {
            for (int x{extent.x}; x <= extent.xMax(); ++x) {
                TilePosition tilePosition{x, y, z};
                Tile* clearedTile{
                    clearTileLayersInternal(tilePosition, layerTypesToClear)};
                if (clearedTile) {
                    layerWasCleared = true;

                    // A layer was cleared. Rebuild the affected tile's
                    // collision.
                    rebuildTileCollision(*clearedTile, tilePosition);
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
        extent, {TileLayer::Type::Terrain, TileLayer::Type::Floor,
                 TileLayer::Type::Wall, TileLayer::Type::Object});
}

void TileMapBase::clear()
{
    chunkExtent = {};
    tileExtent = {};
    chunks.clear();
    tileUpdateHistory.clear();
}

const Chunk* TileMapBase::getChunk(const ChunkPosition& chunkPosition) const
{
    auto chunkIt{chunks.find(chunkPosition)};
    if (chunkIt != chunks.end()) {
        return &(chunkIt->second);
    }

    // The requested chunk is empty or out of bounds.
    return nullptr;
}

const Chunk* TileMapBase::cgetChunk(const ChunkPosition& chunkPosition) const
{
    return getChunk(chunkPosition);
}

const Tile* TileMapBase::getTile(const TilePosition& tilePosition) const
{
    const int CHUNK_WIDTH{static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    // If the chunk doesn't exist, return early.
    ChunkPosition chunkPosition{tilePosition};
    const Chunk* chunk{getChunk(chunkPosition)};
    if (!chunk) {
        //LOG_INFO("Chunk doesn't exist");
        return nullptr;
    }

    // Get the tile's relative position within the chunk.
    int relativeTileX{tilePosition.x - (chunkPosition.x * CHUNK_WIDTH)};
    int relativeTileY{tilePosition.y - (chunkPosition.y * CHUNK_WIDTH)};
    return &(chunk->getTile(relativeTileX, relativeTileY));
}

const Tile* TileMapBase::cgetTile(const TilePosition& tilePosition) const
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
        if (auto tileResult{getTile(*it)}) {
            collisionLocator.updateTile(*it, tileResult->tile);
        }
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

void TileMapBase::loadChunk(const ChunkSnapshot& chunkSnapshot,
                            const ChunkPosition& chunkPosition)
{
    loadChunkInternal(chunkSnapshot, chunkPosition);
}

void TileMapBase::loadChunk(const ChunkWireSnapshot& chunkSnapshot,
                            const ChunkPosition& chunkPosition)
{
    loadChunkInternal(chunkSnapshot, chunkPosition);
}

std::expected<std::reference_wrapper<Chunk>, TileMapBase::ChunkError>
    TileMapBase::getChunk(const ChunkPosition& chunkPosition)
{
    if (!(chunkExtent.contains(chunkPosition))) {
        AM_ASSERT(false, "Failed to get chunk: Invalid chunk position");
        return std::unexpected{ChunkError::InvalidPosition};
    }

    // Find the requested tile's parent chunk. If it doesn't exist, return an 
    // error.
    auto chunkIt{chunks.find(chunkPosition)};
    if (chunkIt == chunks.end()) {
        return std::unexpected{ChunkError::NotFound};
    }

    return chunkIt->second;
}

std::expected<TileMapBase::ChunkTilePair, TileMapBase::ChunkError>
    TileMapBase::getTile(const TilePosition& tilePosition)
{
    ChunkPosition chunkPosition{tilePosition};
    if (auto chunkResult{getChunk(chunkPosition)}) {
        return ChunkTilePair{
            *chunkResult, getTile(*chunkResult, chunkPosition, tilePosition)};
    }
    else if (chunkResult.error() == ChunkError::InvalidPosition) {
        AM_ASSERT(false, "Failed to get tile: Invalid tile position");
        return std::unexpected{chunkResult.error()};
    }
    else {
        return std::unexpected{chunkResult.error()};
    }
}

Tile& TileMapBase::getTile(Chunk& chunk, const ChunkPosition& chunkPosition,
                           const TilePosition& tilePosition)
{
    // Calc the tile's relative position within the chunk.
    const int CHUNK_WIDTH{static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    int relativeTileX{tilePosition.x - (chunkPosition.x * CHUNK_WIDTH)};
    int relativeTileY{tilePosition.y - (chunkPosition.y * CHUNK_WIDTH)};

    return chunk.getTile(relativeTileX, relativeTileY);
}

Chunk* TileMapBase::getOrCreateChunk(const ChunkPosition& chunkPosition)
{
    // Get the chunk.
    if (auto chunkResult{getChunk(chunkPosition)}) {
        // Chunk exists.
        return &(chunkResult->get());
    }
    else if (chunkResult.error() == ChunkError::NotFound) {
        // Chunk doesn't exist, create it.
        auto chunkIt{chunks.emplace(chunkPosition, Chunk{}).first};
        return &(chunkIt->second);
    }
    else if (chunkResult.error() == ChunkError::InvalidPosition) {
        AM_ASSERT(false, "Failed to get chunk: Invalid chunk position");
        return nullptr;
    }

    return nullptr;
}

TileMapBase::ChunkTilePtrPair
    TileMapBase::getOrCreateTile(const TilePosition& tilePosition)
{
    // Get the chunk.
    ChunkPosition chunkPosition{tilePosition};
    Chunk* chunk{};
    if (auto chunkResult{getChunk(chunkPosition)}) {
        // Chunk exists.
        chunk = &(chunkResult->get());
    }
    else if (chunkResult.error() == ChunkError::NotFound) {
        // Chunk doesn't exist, create it.
        auto chunkIt{chunks.emplace(chunkPosition, Chunk{}).first};
        chunk = &(chunkIt->second);
    }
    else if (chunkResult.error() == ChunkError::InvalidPosition) {
        AM_ASSERT(false, "Failed to get tile: Invalid tile position");
        return {nullptr, nullptr};
    }

    // Get the tile.
    Tile& tile{getTile(*chunk, chunkPosition, tilePosition)};

    return {chunk, &tile};
}

Tile* TileMapBase::addTileLayer(const TilePosition& tilePosition,
                                const TileOffset& tileOffset,
                                TileLayer::Type layerType,
                                const GraphicSet& graphicSet,
                                Uint8 graphicValue)
{
    // Get or create the tile, return nullptr if tilePosition is invalid.
    auto [chunk, tile] = getOrCreateTile(tilePosition);
    if (!tile) {
        return nullptr;
    }

    // Add the layer.
    addTileLayer(*chunk, *tile, tileOffset, layerType, graphicSet,
                 graphicValue);

    return tile;
}

void TileMapBase::addTileLayer(Chunk& chunk, Tile& tile,
                               const TileOffset& tileOffset,
                               TileLayer::Type layerType,
                               const GraphicSet& graphicSet, Uint8 graphicValue)
{
    // Add the layer.
    bool layerWasAdded{false};
    if (layerType == TileLayer::Type::Terrain) {
        // If there's an existing terrain, replace it.
        if (TileLayer* terrain{tile.findLayer(TileLayer::Type::Terrain)}) {
            terrain->graphicSet = graphicSet;
            terrain->graphicValue = graphicValue;
        }
        else {
            // No existing terrain, add one.
            tile.addLayer(tileOffset, TileLayer::Type::Terrain, graphicSet,
                          graphicValue);
            layerWasAdded = true;
        }
    }
    else {
        tile.addLayer(tileOffset, layerType, graphicSet, graphicValue);
        layerWasAdded = true;
    }

    // If we added a layer, increment the chunk's count.
    if (layerWasAdded) {
        chunk.tileLayerCount++;
    }
}

void TileMapBase::rebuildTileCollision(Tile& tile, const TilePosition& tilePosition)
{
    // If auto rebuild is enabled, rebuild the affected tile's collision.
    if (autoRebuildCollision) {
        collisionLocator.updateTile(tilePosition, tile);
    }
    else {
        // Not enabled. Queue the affected tile to have its collision rebuilt.
        dirtyCollisionQueue.emplace_back(tilePosition);
    }
}

void TileMapBase::addNorthWall(const TilePosition& tilePosition,
                               const WallGraphicSet& graphicSet)
{
    // If tilePosition isn't a valid position in the map, return early.
    auto [chunk, tile] = getOrCreateTile(tilePosition);
    if (!tile) {
        return;
    }

    // If the tile has a West wall, add a NE gap fill.
    if (tile->findLayer(TileLayer::Type::Wall, Wall::Type::West)) {
        addTileLayer(*chunk, *tile, {}, TileLayer::Type::Wall, graphicSet,
                     Wall::Type::NorthEastGapFill);
    }
    else {
        // No West wall. If there's an existing North wall or NW gap fill, 
        // replace it.
        bool replacedWall{false};
        for (TileLayer& layer : tile->getLayers(TileLayer::Type::Wall)) {
            if ((layer.graphicValue == Wall::Type::North)
                || (layer.graphicValue == Wall::Type::NorthWestGapFill)) {
                layer.graphicSet = graphicSet;
                layer.graphicValue = Wall::Type::North;
                replacedWall = true;
                break;
            }
        }

        // If there was no existing North wall, add one.
        if (!replacedWall) {
            addTileLayer(*chunk, *tile, {}, TileLayer::Type::Wall, graphicSet,
                         Wall::Type::North);
        }
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(*tile, tilePosition);

    /* Add a NW gap fill, if necessary. */
    // If there's a tile to the NE that we might've formed a corner with.
    TilePosition northeastPos{tilePosition.x + 1, tilePosition.y - 1,
                              tilePosition.z};
    const Tile* northeastTile{cgetTile(northeastPos)};
    if (!northeastTile) {
        return;
    };

    // If the NorthEast tile has a West wall.
    const TileLayer* northeastWestWall{
        northeastTile->findLayer(TileLayer::Type::Wall, Wall::Type::West)};
    if (!northeastWestWall) {
        return;
    }

    // We formed a corner. Check if the tile to the East has a wall.
    // Note: We know this tile is in the map bounds cause there's a NorthEast 
    //       tile, but the chunk may not exist yet.
    TilePosition eastPos{tilePosition.x + 1, tilePosition.y,
                         tilePosition.z};
    auto [eastChunk, eastTile] = getOrCreateTile(eastPos);
    if (eastTile->getLayers(TileLayer::Type::Wall).size() == 0) {
        // The East tile has no walls. Add a NorthWestGapFill.
        addTileLayer(*eastChunk, *eastTile, {}, TileLayer::Type::Wall,
                     graphicSet, Wall::Type::NorthWestGapFill);
        rebuildTileCollision(*eastTile, eastPos);
    }
    else if (TileLayer* eastNorthWestGapFill{
                 eastTile->findLayer(TileLayer::Type::Wall,
                                     Wall::Type::NorthWestGapFill)}) {
        // The East tile has a NW gap fill. If its graphic set no longer
        // matches either surrounding wall, make it match the new wall.
        int gapFillID{eastNorthWestGapFill->graphicSet.get().numericID};
        int newNorthID{graphicSet.numericID};
        int westID{northeastWestWall->graphicSet.get().numericID};
        if ((gapFillID != newNorthID) && (gapFillID != westID)) {
            eastNorthWestGapFill->graphicSet = graphicSet;
        }
        rebuildTileCollision(*eastTile, eastPos);
    }
}

void TileMapBase::addWestWall(const TilePosition& tilePosition,
                              const WallGraphicSet& graphicSet)
{
    // If tilePosition isn't a valid position in the map, return early.
    auto [chunk, tile] = getOrCreateTile(tilePosition);
    if (!tile) {
        return;
    }

    // If there's an existing West wall, replace it.
    if (TileLayer* westWall{tile->findLayer(TileLayer::Type::Wall,
                                            Wall::Type::West)}) {
        westWall->graphicSet = graphicSet;
    }
    else {
        // No existing West wall, add one.
        addTileLayer(*chunk, *tile, {}, TileLayer::Type::Wall, graphicSet,
                     Wall::Type::West);
    }

    // If the tile has a North wall, switch it to a NorthEast gap fill.
    if (TileLayer* northWall{tile->findLayer(TileLayer::Type::Wall,
                                             Wall::Type::North)}) {
        // Note: We don't change the graphic set. Only the type changes.
        northWall->graphicValue = Wall::Type::NorthEastGapFill;
    }
    // Else if the tile has a NorthWest gap fill, remove it.
    else {
        remTileLayers(*chunk, *tile, ChunkPosition{tilePosition},
                      TileLayer::Type::Wall, Wall::Type::NorthWestGapFill);
    }

    // Rebuild the affected tile's collision.
    rebuildTileCollision(*tile, tilePosition);

    /* Add a NW gap fill, if necessary. */
    // If there's a tile to the SW that we might've formed a corner with.
    TilePosition southwestPos{tilePosition.x - 1, tilePosition.y + 1,
                              tilePosition.z};
    const Tile* southwestTile{cgetTile(southwestPos)};
    if (!southwestTile) {
        return;
    }

    // If the SouthWest tile has a North wall or a NE gap fill.
    const TileLayer* southwestNorthWall{
        southwestTile->findLayer(TileLayer::Type::Wall, Wall::Type::North)};
    const TileLayer* southwestNorthEastGapFill{southwestTile->findLayer(
        TileLayer::Type::Wall, Wall::Type::NorthEastGapFill)};
    if (!southwestNorthWall && !southwestNorthEastGapFill) {
        return;
    }

    // We formed a corner. Check if the tile to the South has a wall.
    // Note: We know this tile is in the map bounds cause there's a SouthWest 
    //       tile, but the chunk may not exist yet.
    TilePosition southPos{tilePosition.x, tilePosition.y + 1,
                          tilePosition.z};
    auto [southChunk, southTile] = getOrCreateTile(southPos);
    if (southTile->getLayers(TileLayer::Type::Wall).size() == 0) {
        // The South tile has no walls. Add a NorthWestGapFill.
        addTileLayer(*southChunk, *southTile, {}, TileLayer::Type::Wall,
                     graphicSet, Wall::Type::NorthWestGapFill);
    }
    else if (TileLayer* southNorthWestGapFill{southTile->findLayer(
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
    }
    rebuildTileCollision(*southTile, southPos);
}

Tile* TileMapBase::remTileLayer(const TilePosition& tilePosition,
                                const TileOffset& tileOffset,
                                TileLayer::Type layerType, Uint16 graphicSetID,
                                Uint8 graphicValue)
{
    // Get the chunk.
    ChunkPosition chunkPosition{tilePosition};
    Chunk* chunk{};
    if (auto chunkResult{getChunk(chunkPosition)}) {
        // Chunk exists.
        chunk = &(chunkResult->get());
    }
    else {
        return nullptr;
    }

    // Get the tile.
    Tile& tile{getTile(*chunk, chunkPosition, tilePosition)};

    // Remove any matching layers.
    remTileLayer(*chunk, tile, chunkPosition, tileOffset, layerType,
                 graphicSetID, graphicValue);

    return &tile;
}

Tile* TileMapBase::remTileLayer(const TilePosition& tilePosition,
                                TileLayer::Type layerType, Uint16 graphicSetID,
                                Uint8 graphicValue)
{
    // Get the chunk.
    ChunkPosition chunkPosition{tilePosition};
    Chunk* chunk{};
    if (auto chunkResult{getChunk(chunkPosition)}) {
        // Chunk exists.
        chunk = &(chunkResult->get());
    }
    else {
        return nullptr;
    }

    // Get the tile.
    Tile& tile{getTile(*chunk, chunkPosition, tilePosition)};

    // Remove any matching layers.
    remTileLayer(*chunk, tile, chunkPosition, layerType, graphicSetID,
                 graphicValue);

    return &tile;
}

bool TileMapBase::remTileLayer(Chunk& chunk, Tile& tile,
                               const ChunkPosition& chunkPosition,
                               const TileOffset& tileOffset,
                               TileLayer::Type layerType, Uint16 graphicSetID,
                               Uint8 graphicValue)
{
    // Remove any matching layers.
    std::size_t numRemoved{
        tile.removeLayers(tileOffset, layerType, graphicSetID, graphicValue)};

    // If we removed a layer, decrement the chunk's layer count.
    if (numRemoved > 0) {
        AM_ASSERT(chunk.tileLayerCount >= numRemoved,
                  "tileLayerCount was not properly maintained.");
        chunk.tileLayerCount -= static_cast<Uint16>(numRemoved);

        // If the chunk is now completely empty, erase it.
        if (chunk.tileLayerCount == 0) {
            chunks.erase(chunkPosition);
        }

        return true;
    }

    return false;
}

bool TileMapBase::remTileLayer(Chunk& chunk, Tile& tile,
                               const ChunkPosition& chunkPosition,
                               TileLayer::Type layerType, Uint16 graphicSetID,
                               Uint8 graphicValue)
{
    // Remove any matching layers.
    std::size_t numRemoved{
        tile.removeLayers(layerType, graphicSetID, graphicValue)};

    // If we removed a layer, decrement the chunk's layer count.
    if (numRemoved > 0) {
        AM_ASSERT(chunk.tileLayerCount >= numRemoved,
                  "tileLayerCount was not properly maintained.");
        chunk.tileLayerCount -= static_cast<Uint16>(numRemoved);

        // If the chunk is now completely empty, erase it.
        if (chunk.tileLayerCount == 0) {
            chunks.erase(chunkPosition);
        }

        return true;
    }

    return false;
}

bool TileMapBase::remTileLayers(Chunk& chunk, Tile& tile,
                                const ChunkPosition& chunkPosition,
                                TileLayer::Type layerType, Uint8 graphicValue)
{
    // Remove any matching layers.
    std::size_t numRemoved{tile.removeLayers(layerType, graphicValue)};

    // If we removed a layer, decrement the chunk's layer count.
    if (numRemoved > 0) {
        AM_ASSERT(chunk.tileLayerCount >= numRemoved,
                  "tileLayerCount was not properly maintained.");
        chunk.tileLayerCount -= static_cast<Uint16>(numRemoved);

        // If the chunk is now completely empty, erase it.
        if (chunk.tileLayerCount == 0) {
            chunks.erase(chunkPosition);
        }

        return true;
    }

    return false;
}

bool TileMapBase::remNorthWall(const TilePosition& tilePosition)
{
    auto tileResult{getTile(tilePosition)};
    if (!tileResult) {
        return false;
    }
    auto [chunk, tile] = *tileResult;

    // If the tile has a North wall or NE gap fill, remove it.
    ChunkPosition chunkPosition{tilePosition};
    bool wallWasRemoved{remTileLayers(
        chunk, tile, chunkPosition, TileLayer::Type::Wall, Wall::Type::North)};
    if (!wallWasRemoved) {
        wallWasRemoved
            = remTileLayers(chunk, tile, chunkPosition, TileLayer::Type::Wall,
                            Wall::Type::NorthEastGapFill);
    }

    // If a wall was removed.
    if (wallWasRemoved) {
        // Rebuild the affected tile's collision.
        rebuildTileCollision(tile, tilePosition);

        // Check if there's a NW gap fill to the East.
        TilePosition eastTilePosition{tilePosition.x + 1, tilePosition.y,
                                      tilePosition.z};
        if (auto eastTileResult{getTile(eastTilePosition)}) {
            auto [eastChunk, eastTile] = *eastTileResult;

            // If the East tile has a gap fill for a corner that we just broke,
            // remove it.
            if (remTileLayers(
                    eastChunk, eastTile, ChunkPosition{eastTilePosition},
                    TileLayer::Type::Wall, Wall::Type::NorthWestGapFill)) {
                rebuildTileCollision(eastTile, tilePosition);
            }
        }
    }

    return wallWasRemoved;
}

bool TileMapBase::remWestWall(const TilePosition& tilePosition)
{
    auto tileResult{getTile(tilePosition)};
    if (!tileResult) {
        return false;
    }
    auto [chunk, tile] = *tileResult;

    // If the tile has a West wall, remove it.
    ChunkPosition chunkPosition{tilePosition};
    if (remTileLayers(chunk, tile, chunkPosition, TileLayer::Type::Wall,
                      Wall::Type::West)) {
        // If the tile has a NE gap fill, change it to a North.
        if (TileLayer* northEastGapFill{tile.get().findLayer(
                TileLayer::Type::Wall, Wall::Type::NorthEastGapFill)}) {
            northEastGapFill->graphicValue = Wall::Type::North;
        }

        // Rebuild the affected tile's collision.
        rebuildTileCollision(tile, tilePosition);

        // Check if there's a NW gap fill to the South
        TilePosition southTilePosition{tilePosition.x, tilePosition.y + 1,
                                       tilePosition.z};
        if (auto southTileResult{getTile(southTilePosition)}) {
            auto [southChunk, southTile] = *southTileResult;

            // If the South tile has a gap fill for a corner that we just broke,
            // remove it.
            if (remTileLayers(
                    southChunk, southTile, ChunkPosition{southTilePosition},
                    TileLayer::Type::Wall, Wall::Type::NorthWestGapFill)) {
                rebuildTileCollision(southTile, tilePosition);
            }
        }

        return true;
    }

    return false;
}

Tile* TileMapBase::clearTileLayersInternal(
    const TilePosition& tilePosition,
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    Chunk* chunk{};
    Tile* tile{};
    if (auto tileResult{getTile(tilePosition)}) {
        // Tile exists.
        chunk = &(tileResult->chunk.get());
        tile = &(tileResult->tile.get());
    }
    else {
        return nullptr;
    }

    // If we're being asked to clear every layer, clear the whole tile.
    std::size_t numRemoved{0};
    if (layerTypesToClear[TileLayer::Type::Terrain]
        && layerTypesToClear[TileLayer::Type::Floor]
        && layerTypesToClear[TileLayer::Type::Wall]
        && layerTypesToClear[TileLayer::Type::Object]) {
        numRemoved = tile->clear();
    }
    else {
        numRemoved = tile->clearLayers(layerTypesToClear);
    }

    // If we cleared any layers, decrement the chunk's layer count.
    if (numRemoved > 0) {
        AM_ASSERT(chunk->tileLayerCount >= numRemoved,
                  "tileLayerCount was not properly maintained.");
        chunk->tileLayerCount -= static_cast<Uint16>(numRemoved);

        // If the chunk is now completely empty, erase it.
        if (chunk->tileLayerCount == 0) {
            chunks.erase(ChunkPosition{tilePosition});
        }

        return tile;
    }

    return nullptr;
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

template<typename T>
void TileMapBase::loadChunkInternal(const T& chunkSnapshot,
                                    const ChunkPosition& chunkPosition)
{
    // Note: We can't use the set/add functions because they'll push updates
    //       into the history, and addWall() adds extra walls.

    // If the chunk doesn't exist, create it.
    Chunk* chunk{getOrCreateChunk(chunkPosition)};
    if (!chunk) {
        LOG_FATAL("Invalid chunk position.");
        return;
    }

    // Iterate each of the tiles in the chunk snapshot.
    std::size_t currentTileLayerStartIndex{0};
    std::size_t currentTileIndex{0};
    std::size_t currentTileOffsetIndex{0};
    for (Uint8 tileLayerCount : chunkSnapshot.tileLayerCounts) {
        Tile& tile{chunk->tiles[currentTileIndex]};
        tile.clear();
        bool rebuildCollision{false};

        // Add each of this tile's layers to the map.
        for (std::size_t i{0}; i < tileLayerCount; ++i) {
            Uint8 paletteIndex{
                chunkSnapshot.tileLayers[currentTileLayerStartIndex + i]};
            const auto& paletteEntry{chunkSnapshot.palette[paletteIndex]};

            // Get this layer's graphic set and (if Floor/Object) tile offset.
            const GraphicSet* graphicSet{nullptr};
            TileOffset tileOffset{};
            switch (paletteEntry.layerType) {
                case TileLayer::Type::Terrain: {
                    graphicSet = &(graphicData.getTerrainGraphicSet(
                        paletteEntry.graphicSetID));
                    rebuildCollision = true;
                    break;
                }
                case TileLayer::Type::Floor: {
                    graphicSet = &(graphicData.getFloorGraphicSet(
                        paletteEntry.graphicSetID));
                    tileOffset
                        = chunkSnapshot.tileOffsets[currentTileOffsetIndex++];
                    break;
                }
                case TileLayer::Type::Wall: {
                    graphicSet = &(graphicData.getWallGraphicSet(
                        paletteEntry.graphicSetID));
                    rebuildCollision = true;
                    break;
                }
                case TileLayer::Type::Object: {
                    graphicSet = &(graphicData.getObjectGraphicSet(
                        paletteEntry.graphicSetID));
                    tileOffset
                        = chunkSnapshot.tileOffsets[currentTileOffsetIndex++];
                    rebuildCollision = true;
                    break;
                }
                default: {
                    break;
                }
            }

            if (!graphicSet) {
                LOG_FATAL("Graphic set was not found for loaded tile layer.");
            }

            // Add the layer to the tile.
            addTileLayer(*chunk, tile, tileOffset, paletteEntry.layerType,
                         *graphicSet, paletteEntry.graphicValue);
        }

        // Rebuild the tile's collision if necessary.
        if (rebuildCollision) {
            Morton::Result2D xyOffsets{
                Morton::decode16x16(static_cast<Uint8>(currentTileIndex))};
            TilePosition tilePosition(chunkPosition);
            tilePosition.x += xyOffsets.x;
            tilePosition.y += xyOffsets.y;
            rebuildTileCollision(tile, tilePosition);
        }

        currentTileLayerStartIndex += tileLayerCount;
        currentTileIndex++;
    }
}

} // End namespace AM
