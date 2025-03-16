#include "EntityLocator.h"
#include "SharedConfig.h"
#include "Position.h"
#include "Cylinder.h"
#include "BoundingBox.h"
#include "Collision.h"
#include "CellPosition.h"
#include "TilePosition.h"
#include "Log.h"
#include "AMAssert.h"
#include "entt/entity/registry.hpp"
#include <cmath>
#include <algorithm>

namespace AM
{
EntityLocator::EntityLocator(entt::registry& inRegistry)
: registry{inRegistry}
, gridCellExtent{}
, entityGrid{}
, entityMap{}
, returnVector{}
{
}

void EntityLocator::setGridSize(const TileExtent& mapTileExtent)
{
    // Set our grid extent to match the tile map.
    gridCellExtent
        = CellExtent(mapTileExtent, SharedConfig::ENTITY_LOCATOR_CELL_WIDTH,
                     SharedConfig::ENTITY_LOCATOR_CELL_HEIGHT);

    // Resize the grid to fit the map.
    entityGrid.resize(linearizeCellIndex(gridCellExtent.max()) + 1);
}

void EntityLocator::updateEntity(entt::entity entity, const Position& position)
{
    // Find the cell that the entity's position intersects.
    CellPosition cellPosition(position, CELL_WORLD_WIDTH, CELL_WORLD_HEIGHT);
    if (!(gridCellExtent.contains(cellPosition))) {
        LOG_ERROR("Tried to track entity that is outside of the locator's "
                  "grid: (%d, %d, %d)ce.",
                  cellPosition.x, cellPosition.y, cellPosition.z);
        return;
    }

    // If we're already tracking this entity.
    auto entityIt{entityMap.find(entity)};
    if (entityIt != entityMap.end()) {
        // If the cell position hasn't changed, exit early.
        if (cellPosition == entityIt->second) {
            return;
        }
        else {
            // Cell position isn't the same. Remove the entity from the old 
            // cell.
            clearEntityFromCell(entity, entityIt->second);
        }
    }

    // Add the entity to the cell.
    std::size_t linearizedIndex{linearizeCellIndex(cellPosition)};
    std::vector<entt::entity>& cell{entityGrid[linearizedIndex]};

    cell.push_back(entity);

    // Update the entity map.
    entityMap[entity] = cellPosition;
}

void EntityLocator::removeEntity(entt::entity entity)
{
    // If we aren't already tracking this entity, error.
    auto entityIt{entityMap.find(entity)};
    if (entityIt == entityMap.end()) {
        // Note: Since every entity has a position, we expect them to always 
        //       be in this locator.
        LOG_ERROR("Tried to remove entity that wasn't added to this locator.");
        return;
    }

    // Remove the entity from cell that it's located in.
    clearEntityFromCell(entity, entityIt->second);

    // Remove the entity from the map.
    entityMap.erase(entityIt);
}

std::vector<entt::entity>& EntityLocator::getEntities(const Cylinder& cylinder)
{
    AM_ASSERT(cylinder.radius >= 0, "Cylinder can't have negative radius.");
    AM_ASSERT(cylinder.halfHeight >= 0,
              "Cylinder can't have negative half height.");

    // Perform a broad phase.
    getEntitiesBroad(cylinder);

    // Erase any entities whose position isn't within the cylinder.
    std::erase_if(returnVector, [this, &cylinder](entt::entity entity) {
        const Position& position{registry.get<Position>(entity)};
        return !(cylinder.intersects(position));
    });

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getEntities(const TileExtent& tileExtent)
{
    // Perform a broad phase.
    getEntitiesBroad(tileExtent);

    // Erase any entities that don't actually intersect the extent.
    std::erase_if(returnVector, [this, &tileExtent](entt::entity entity) {
        const Position& position{registry.get<Position>(entity)};
        return !(tileExtent.contains(position));
    });

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getEntities(const ChunkExtent& chunkExtent)
{
    // Convert to TileExtent.
    TileExtent tileExtent{chunkExtent};

    return getEntities(tileExtent);
}

std::vector<entt::entity>&
    EntityLocator::getEntitiesBroad(const Cylinder& cylinder)
{
    // Clear the return vector.
    returnVector.clear();

    // Calc the cell extent that is intersected by the cylinder.
    CellExtent cylinderCellExtent(cylinder, CELL_WORLD_WIDTH,
                                  CELL_WORLD_HEIGHT);

    // Clip the extent to the grid's bounds.
    cylinderCellExtent = cylinderCellExtent.intersectWith(gridCellExtent);

    // Add the entities in every intersected cell to the return vector.
    for (int z{cylinderCellExtent.z}; z <= cylinderCellExtent.zMax(); ++z) {
        for (int y{cylinderCellExtent.y}; y <= cylinderCellExtent.yMax(); ++y) {
            for (int x{cylinderCellExtent.x}; x <= cylinderCellExtent.xMax();
                 ++x) {
                // Add the entities in this cell to the return vector.
                std::size_t linearizedIndex{linearizeCellIndex({x, y, z})};
                std::vector<entt::entity>& entityVec{
                    entityGrid[linearizedIndex]};
                returnVector.insert(returnVector.end(), entityVec.begin(),
                                    entityVec.end());
            }
        }
    }

    // Note: We don't need to de-duplicate since an entity's Position will only 
    //       ever be in one cell at a time.

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getEntitiesBroad(const BoundingBox& boundingBox)
{
    // Convert to TileExtent.
    return getEntitiesBroad(TileExtent(boundingBox));
}

std::vector<entt::entity>&
    EntityLocator::getEntitiesBroad(const TileExtent& tileExtent)
{
    // Clear the return vector.
    returnVector.clear();

    // Calc the cell extent that is intersected by the tile extent.
    CellExtent tileCellExtent(tileExtent,
                              SharedConfig::ENTITY_LOCATOR_CELL_WIDTH,
                              SharedConfig::ENTITY_LOCATOR_CELL_HEIGHT);

    // Clip the extent to the grid's bounds.
    tileCellExtent = tileCellExtent.intersectWith(gridCellExtent);

    // Add the entities in every intersected cell to the return vector.
    for (int z{tileCellExtent.z}; z <= tileCellExtent.zMax(); ++z) {
        for (int y{tileCellExtent.y}; y <= tileCellExtent.yMax(); ++y) {
            for (int x{tileCellExtent.x}; x <= tileCellExtent.xMax(); ++x) {
                // Add the entities in this cell to the return vector.
                std::size_t linearizedIndex{linearizeCellIndex({x, y, z})};
                std::vector<entt::entity>& entityVec{
                    entityGrid[linearizedIndex]};
                returnVector.insert(returnVector.end(), entityVec.begin(),
                                    entityVec.end());
            }
        }
    }

    // Note: We don't need to de-duplicate since an entity's Position will only 
    //       ever be in one cell at a time.

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getEntitiesBroad(const ChunkExtent& chunkExtent)
{
    // Convert to TileExtent.
    TileExtent tileExtent{chunkExtent};

    return getEntitiesBroad(tileExtent);
}

void EntityLocator::clearEntityFromCell(entt::entity entity,
                                        const CellPosition& clearPosition)
{
    // Find and erase the entity from the cell.
    std::size_t linearizedIndex{linearizeCellIndex(clearPosition)};
    std::vector<entt::entity>& cell{entityGrid[linearizedIndex]};

    auto entityIt{std::find(cell.begin(), cell.end(), entity)};
    if (entityIt != cell.end()) {
        cell.erase(entityIt);
    }
}

} // End namespace AM
