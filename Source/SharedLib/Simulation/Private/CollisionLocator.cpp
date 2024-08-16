#include "CollisionLocator.h"
#include "SharedConfig.h"
#include "Tile.h"
#include "Cylinder.h"
#include "BoundingBox.h"
#include "MinMaxBox.h"
#include "Collision.h"
#include "CellPosition.h"
#include "Transforms.h"
#include "Log.h"
#include "AMAssert.h"
#include <cmath>
#include <algorithm>

namespace AM
{
CollisionLocator::CollisionLocator()
: gridCellExtent{}
, collisionVolumes{}
, freeCollisionVolumesIndices{}
, collisionGrid{}
, entityMap{}
, tileMap{}
, terrainGrid{}
, indexVector{}
, returnVector{}
{
}

void CollisionLocator::setGridSize(const TileExtent& mapTileExtent)
{
    // Set our grid extent to match the tile map.
    gridCellExtent
        = CellExtent(mapTileExtent, SharedConfig::COLLISION_LOCATOR_CELL_WIDTH,
                     SharedConfig::COLLISION_LOCATOR_CELL_HEIGHT);

    // Resize the grid to fit the map.
    collisionGrid.resize(linearizeCellIndex(gridCellExtent.max()) + 1);
}

void CollisionLocator::updateEntity(entt::entity entity,
                                    const BoundingBox& collisionVolume,
                                    CollisionObjectType::Value objectType)
{
    // Find the cells that the collision volume intersects.
    CellExtent cellExtent(collisionVolume, CELL_WORLD_WIDTH, CELL_WORLD_HEIGHT);
    if (!(gridCellExtent.contains(cellExtent))) {
        LOG_ERROR("Tried to track collision that is outside of the locator's "
                  "grid: (%d, %d, %d, %d, %d, %d)ce.",
                  cellExtent.x, cellExtent.y, cellExtent.z, cellExtent.xLength,
                  cellExtent.yLength, cellExtent.zLength);
        return;
    }

    // If we're already tracking this entity.
    auto entityIt{entityMap.find(entity)};
    Uint16 volumeIndex{};
    if (entityIt != entityMap.end()) {
        CollisionInfo& volumeInfo{collisionVolumes[entityIt->second]};
        CellExtent oldCellExtent(volumeInfo.collisionVolume, CELL_WORLD_WIDTH,
                                 CELL_WORLD_HEIGHT);
        
        // Update their collision volume.
        volumeIndex = entityIt->second;
        collisionVolumes[volumeIndex].collisionVolume = collisionVolume;
        collisionVolumes[volumeIndex].objectType = objectType;
        collisionVolumes[volumeIndex].entity = entity;

        // If the cell extent hasn't changed, exit early.
        if (cellExtent == oldCellExtent) {
            return;
        }
        else {
            // Cell extent isn't the same. Remove the entity from the old
            // cells.
            clearCollisionVolumeFromCells(volumeIndex, oldCellExtent);
        }
    }
    else {
        // New entity. If we have a free volume vector index, use it.
        if (!(freeCollisionVolumesIndices.empty())) {
            volumeIndex = freeCollisionVolumesIndices.back();
            freeCollisionVolumesIndices.pop_back();

            collisionVolumes[volumeIndex].collisionVolume = collisionVolume;
            collisionVolumes[volumeIndex].objectType = objectType;
            collisionVolumes[volumeIndex].entity = entity;
        }
        else {
            // No free indices, add the volume to the back.
            collisionVolumes.emplace_back(collisionVolume, objectType, entity);
            volumeIndex = static_cast<Uint16>(collisionVolumes.size() - 1);
        }

        // Add the new entity to the map.
        entityMap[entity] = volumeIndex;
    }

    // Add the entity's collision volume to the grid.
    addCollisionVolumeToCells(volumeIndex, cellExtent);
}

void CollisionLocator::updateTile(const TilePosition& tilePosition,
                                  const Tile& tile)
{
    // If we're already tracking this tile, clear its old collision data.
    auto tileIt{tileMap.find(tilePosition)};
    if (tileIt != tileMap.end()) {
        // For each layer that was in the tile.
        for (Uint16 volumeIndex : tileIt->second) {
            CollisionInfo& volumeInfo{collisionVolumes[volumeIndex]};

            // Clear it from the grid.
            CellExtent cellExtent(volumeInfo.collisionVolume, CELL_WORLD_WIDTH,
                                  CELL_WORLD_HEIGHT);
            clearCollisionVolumeFromCells(volumeIndex, cellExtent);
            
            // Mark its index as now being free.
            freeCollisionVolumesIndices.push_back(volumeIndex);
        }

        // If the tile has no layers, erase it from the map and return early.
        if (tile.getAllLayers().empty()) {
            tileMap.erase(tileIt);
            return;
        }
    }

    // Add the tile to the map, or clear if it was already present.
    std::vector<Uint16>& tileLayerCollisionIndices{tileMap[tilePosition]};
    tileLayerCollisionIndices.clear();

    // Add all of this tile's layers that have collision to the grid.
    float terrainHeight{0};
    for (const TileLayer& layer : tile.getAllLayers()) {
        GraphicRef graphic{layer.getGraphic()};

        // If it's terrain, generate collision for it.
        // (We ignore modelBounds and collisionEnabled on terrain, all terrain 
        // gets generated collision). 
        BoundingBox collisionVolume{};
        if (layer.type == TileLayer::Type::Terrain) {
            collisionVolume = Terrain::calcWorldBounds(
                tilePosition, static_cast<Terrain::Value>(layer.graphicValue));
            terrainHeight
                = collisionVolume.center.z + collisionVolume.halfExtents.z;
        }
        // If it's a floor, skip it (they never have collision).
        else if (layer.type == TileLayer::Type::Floor) {
            continue;
        }
        // If it's a wall or object, add its assigned collision.
        else if (graphic.getCollisionEnabled()) {
            collisionVolume = Transforms::modelToWorldTile(
                graphic.getModelBounds(), tilePosition);

            // If it's a wall, add the terrain height.
            if (layer.type == TileLayer::Type::Wall) {
                collisionVolume.center.z += terrainHeight;
            }
        }

        // If we have a free volume vector index, use it.
        Uint16 volumeIndex{};
        if (!(freeCollisionVolumesIndices.empty())) {
            volumeIndex = freeCollisionVolumesIndices.back();
            freeCollisionVolumesIndices.pop_back();

            collisionVolumes[volumeIndex].collisionVolume = collisionVolume;
            collisionVolumes[volumeIndex].objectType
                = CollisionObjectType::TileLayer;
        }
        else {
            // No free indices, add the volume to the back.
            collisionVolumes.emplace_back(collisionVolume,
                                          CollisionObjectType::TileLayer);
            volumeIndex = static_cast<Uint16>(collisionVolumes.size() - 1);
        }

        // Convert the volume to a cell extent and make sure each length is 
        // non-zero (it's fine for the volume to be a plane, but if the cell 
        // extent has any zero lengths, this layer won't be added to any cells).
        CellExtent cellExtent(collisionVolume, CELL_WORLD_WIDTH,
                              CELL_WORLD_HEIGHT);
        cellExtent.xLength = std::max(cellExtent.xLength, 1);
        cellExtent.yLength = std::max(cellExtent.yLength, 1);
        cellExtent.zLength = std::max(cellExtent.zLength, 1);
        
        // Add this layer's collision volume to the grid.
        addCollisionVolumeToCells(volumeIndex, cellExtent);

        // Add this layer's index to the map.
        tileLayerCollisionIndices.push_back(volumeIndex);
    }
}

void CollisionLocator::removeEntity(entt::entity entity)
{
    // If we aren't already tracking this entity, do nothing.
    auto entityIt{entityMap.find(entity)};
    if (entityIt == entityMap.end()) {
        LOG_ERROR("Tried to remove entity that wasn't added to this locator.");
        return;
    }

    // Remove the entity from the cells that it's located in.
    const CollisionInfo& volumeInfo{
        collisionVolumes[entityIt->second]};
    CellExtent cellExtent(volumeInfo.collisionVolume, CELL_WORLD_WIDTH,
                          CELL_WORLD_HEIGHT);
    clearCollisionVolumeFromCells(entityIt->second, cellExtent);

    // Mark its index as now being free.
    freeCollisionVolumesIndices.push_back(entityIt->second);

    // Remove the entity from the map.
    entityMap.erase(entityIt);
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisions(const Cylinder& cylinder,
                                    CollisionObjectTypeMask objectTypeMask)
{
    AM_ASSERT(cylinder.radius >= 0, "Cylinder can't have negative radius.");

    // Perform a broad phase.
    getCollisionsBroad(cylinder, objectTypeMask);

    // Erase any volumes that don't actually intersect the extent.
    std::erase_if(returnVector,
                  [this, &cylinder](const CollisionInfo* otherInfo) {
                      return !(otherInfo->collisionVolume.intersects(cylinder));
                  });

    return returnVector;
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisions(const BoundingBox& boundingBox,
                                    CollisionObjectTypeMask objectTypeMask)
{
    // Perform a broad phase.
    getCollisionsBroad(boundingBox, objectTypeMask);

    // Erase any volumes that don't actually intersect the extent.
    std::erase_if(
        returnVector, [this, &boundingBox](const CollisionInfo* otherInfo) {
            return !(otherInfo->collisionVolume.intersects(boundingBox));
        });

    return returnVector;
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisions(const TileExtent& tileExtent,
                                    CollisionObjectTypeMask objectTypeMask)
{
    // Perform a broad phase.
    getCollisionsBroad(tileExtent, objectTypeMask);

    // Erase any volumes that don't actually intersect the extent.
    std::erase_if(
        returnVector, [this, &tileExtent](const CollisionInfo* otherInfo) {
            return !(otherInfo->collisionVolume.intersects(tileExtent));
        });

    return returnVector;
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisions(const ChunkExtent& chunkExtent,
                                    CollisionObjectTypeMask objectTypeMask)
{
    // Convert to TileExtent.
    return getCollisions(TileExtent(chunkExtent), objectTypeMask);
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisionsBroad(const Cylinder& cylinder,
                                         CollisionObjectTypeMask objectTypeMask)
{
    // Calc the cell extent that is intersected by the cylinder.
    CellExtent cylinderCellExtent(cylinder, CELL_WORLD_WIDTH,
                                  CELL_WORLD_HEIGHT);

    // Clip the extent to the grid's bounds.
    cylinderCellExtent.intersectWith(gridCellExtent);

    // Add the indices in every intersected cell to the scratch vector.
    indexVector.clear();
    for (int z{cylinderCellExtent.z}; z <= cylinderCellExtent.zMax(); ++z) {
        for (int y{cylinderCellExtent.y}; y <= cylinderCellExtent.yMax(); ++y) {
            for (int x{cylinderCellExtent.x}; x <= cylinderCellExtent.xMax();
                 ++x) {
                std::size_t linearizedIndex{linearizeCellIndex({x, y, z})};
                std::vector<Uint16>& cell{collisionGrid[linearizedIndex]};
                indexVector.insert(indexVector.end(), cell.begin(), cell.end());
            }
        }
    }

    // Sort and remove duplicates from the scratch vector.
    std::sort(indexVector.begin(), indexVector.end());
    indexVector.erase(std::unique(indexVector.begin(), indexVector.end()),
                      indexVector.end());

    // Push the collision volumes into the return vector.
    returnVector.clear();
    for (Uint16 volumeIndex : indexVector) {
        const CollisionInfo& volumeInfo{collisionVolumes[volumeIndex]};

        // Filter out any objects that don't match the mask.
        if (volumeInfo.objectType & objectTypeMask) {
            returnVector.push_back(&volumeInfo);
        }
    }

    return returnVector;
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisionsBroad(const BoundingBox& boundingBox,
                                         CollisionObjectTypeMask objectTypeMask)
{
    // Convert to TileExtent.
    return getCollisionsBroad(TileExtent(boundingBox), objectTypeMask);
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisionsBroad(const MinMaxBox& boundingBox,
                                         CollisionObjectTypeMask objectTypeMask)
{
    // Convert to TileExtent.
    return getCollisionsBroad(TileExtent(boundingBox), objectTypeMask);
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisionsBroad(const TileExtent& tileExtent,
                                         CollisionObjectTypeMask objectTypeMask)
{
    // Calc the cell extent that is intersected by the tile extent.
    CellExtent tileCellExtent(tileExtent,
                              SharedConfig::ENTITY_LOCATOR_CELL_WIDTH,
                              SharedConfig::ENTITY_LOCATOR_CELL_HEIGHT);

    // Clip the extent to the grid's bounds.
    tileCellExtent.intersectWith(gridCellExtent);

    // Add the indices in every intersected cell to the scratch vector.
    indexVector.clear();
    for (int z{tileCellExtent.z}; z <= tileCellExtent.zMax(); ++z) {
        for (int y{tileCellExtent.y}; y <= tileCellExtent.yMax(); ++y) {
            for (int x{tileCellExtent.x}; x <= tileCellExtent.xMax(); ++x) {
                std::size_t linearizedIndex{linearizeCellIndex({x, y, z})};
                std::vector<Uint16>& cell{collisionGrid[linearizedIndex]};
                indexVector.insert(indexVector.end(), cell.begin(), cell.end());
            }
        }
    }

    // Sort and remove duplicates from the scratch vector.
    std::sort(indexVector.begin(), indexVector.end());
    indexVector.erase(std::unique(indexVector.begin(), indexVector.end()),
                      indexVector.end());

    // Push the collision volumes into the return vector.
    returnVector.clear();
    for (Uint16 volumeIndex : indexVector) {
        const CollisionInfo& volumeInfo{collisionVolumes[volumeIndex]};

        // Filter out any objects that don't match the mask.
        if (volumeInfo.objectType & objectTypeMask) {
            returnVector.push_back(&volumeInfo);
        }
    }

    return returnVector;
}

std::vector<const CollisionLocator::CollisionInfo*>&
    CollisionLocator::getCollisionsBroad(const ChunkExtent& chunkExtent,
                                         CollisionObjectTypeMask objectTypeMask)
{
    // Convert to TileExtent.
    return getCollisionsBroad(TileExtent(chunkExtent), objectTypeMask);
}

void CollisionLocator::addCollisionVolumeToCells(Uint16 volumeIndex,
                                                 const CellExtent& cellExtent)
{
    // Add the volume's index to all the cells that it occupies.
    // Note: We allow for duplicate collision volumes.
    for (int z{cellExtent.z}; z <= cellExtent.zMax(); ++z) {
        for (int y{cellExtent.y}; y <= cellExtent.yMax(); ++y) {
            for (int x{cellExtent.x}; x <= cellExtent.xMax(); ++x) {
                // Add the volume's index to this cell's vector.
                std::size_t linearizedIndex{linearizeCellIndex({x, y, z})};
                std::vector<Uint16>& cell{collisionGrid[linearizedIndex]};

                cell.push_back(volumeIndex);
            }
        }
    }
}

void CollisionLocator::clearCollisionVolumeFromCells(
    Uint16 volumeIndex, const CellExtent& clearExtent)
{
    // Iterate through all the cells that the volume occupies.
    for (int z{clearExtent.z}; z <= clearExtent.zMax(); ++z) {
        for (int y{clearExtent.y}; y <= clearExtent.yMax(); ++y) {
            for (int x{clearExtent.x}; x <= clearExtent.xMax(); ++x) {
                // Find and erase the volume's index from this cell.
                std::size_t linearizedIndex{linearizeCellIndex({x, y, z})};
                std::vector<Uint16>& cell{collisionGrid[linearizedIndex]};

                auto indexIt{std::find(cell.begin(), cell.end(), volumeIndex)};
                if (indexIt != cell.end()) {
                    cell.erase(indexIt);
                }
            }
        }
    }
}

} // End namespace AM
