#include "EntityLocator.h"
#include "SharedConfig.h"
#include "Position.h"
#include "Cylinder.h"
#include "BoundingBox.h"
#include "Collision.h"
#include "CellPosition.h"
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
, cellWorldWidth{SharedConfig::CELL_WIDTH * SharedConfig::TILE_WORLD_WIDTH}
{
}

void EntityLocator::setGridSize(std::size_t inMapXLengthTiles,
                                std::size_t inMapYLengthTiles)
{
    if (((inMapXLengthTiles % SharedConfig::CELL_WIDTH) != 0)
        || ((inMapYLengthTiles % SharedConfig::CELL_WIDTH) != 0)) {
        LOG_FATAL("Map length must be divisible by CELL_WIDTH");
    }

    // Set our grid size to match the tile map.
    gridCellExtent.xLength
        = static_cast<int>(inMapXLengthTiles / SharedConfig::CELL_WIDTH);
    gridCellExtent.yLength
        = static_cast<int>(inMapYLengthTiles / SharedConfig::CELL_WIDTH);

    // Resize the grid to fit the map.
    entityGrid.resize(gridCellExtent.xLength * gridCellExtent.yLength);
}

void EntityLocator::setEntityLocation(entt::entity entity,
                                      const BoundingBox& boundingBox)
{
    // Find the cells that the bounding box intersects.
    CellExtent boxCellExtent{};
    boxCellExtent.x
        = static_cast<int>(std::floor(boundingBox.minX / cellWorldWidth));
    boxCellExtent.y
        = static_cast<int>(std::floor(boundingBox.minY / cellWorldWidth));
    boxCellExtent.xLength
        = (static_cast<int>(std::ceil(boundingBox.maxX / cellWorldWidth))
           - boxCellExtent.x);
    boxCellExtent.yLength
        = (static_cast<int>(std::ceil(boundingBox.maxY / cellWorldWidth))
           - boxCellExtent.y);

    if (!(gridCellExtent.containsExtent(boxCellExtent))) {
        LOG_FATAL("Tried to track entity that is outside of the locator's "
                  "grid: (%d, %d, %d, %d)ce.",
                  boxCellExtent.x, boxCellExtent.y, boxCellExtent.xLength,
                  boxCellExtent.yLength);
    }

    // If we already have a location for the entity, clear it.
    auto entityIt{entityMap.find(entity)};
    if (entityIt != entityMap.end()) {
        // Clear the entity's current location.
        clearEntityLocation(entity, entityIt->second);
    }

    // Add the entity to the map, or update its extent if it already exists.
    entityMap.insert_or_assign(entity, boxCellExtent);

    // Add the entity to all the cells that it occupies.
    for (int x = boxCellExtent.x; x <= boxCellExtent.xMax(); ++x) {
        for (int y = boxCellExtent.y; y <= boxCellExtent.yMax(); ++y) {
            // Add the entity to this cell's entity vector.
            std::size_t linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityVec{entityGrid[linearizedIndex]};

            entityVec.push_back(entity);
        }
    }
}

std::vector<entt::entity>&
    EntityLocator::getEntities(const Cylinder& cylinder)
{
    AM_ASSERT(cylinder.radius >= 0, "Cylinder can't have negative radius.");

    // Run a coarse pass.
    getEntitiesCoarse(cylinder);

    // Erase any entities whose position isn't within the cylinder.
    std::erase_if(
        returnVector, [this, &cylinder](entt::entity entity) {
            const Position& position{registry.get<Position>(entity)};
            return !(cylinder.intersects(position));
        });

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getEntities(const TileExtent& tileExtent)
{
    // Run a coarse pass.
    getEntitiesCoarse(tileExtent);

    // Erase any entities that don't actually intersect the extent.
    std::erase_if(returnVector, [this, &tileExtent](entt::entity entity) {
        const Position& position{registry.get<Position>(entity)};
        return !(tileExtent.containsPosition(position.asTilePosition()));
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
    EntityLocator::getCollisions(const Cylinder& cylinder)
{
    AM_ASSERT(cylinder.radius >= 0, "Cylinder can't have negative radius.");

    // Run a coarse pass.
    getEntitiesCoarse(cylinder);

    // Erase any entities that don't actually intersect the cylinder.
    std::erase_if(
        returnVector, [this, &cylinder](entt::entity entity) {
            const Collision& collision{registry.get<Collision>(entity)};
            return !(collision.worldBounds.intersects(cylinder));
        });

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getCollisions(const BoundingBox& boundingBox)
{
    // Run a coarse pass.
    getEntitiesCoarse(boundingBox);

    // Erase any entities that don't actually intersect the extent.
    std::erase_if(returnVector, [this, &boundingBox](entt::entity entity) {
        const Collision& collision{registry.get<Collision>(entity)};
        return !(collision.worldBounds.intersects(boundingBox));
    });

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getCollisions(const TileExtent& tileExtent)
{
    // Run a coarse pass.
    getEntitiesCoarse(tileExtent);

    // Erase any entities that don't actually intersect the extent.
    std::erase_if(returnVector, [this, &tileExtent](entt::entity entity) {
        const Collision& collision{registry.get<Collision>(entity)};
        return !(collision.worldBounds.intersects(tileExtent));
    });

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getCollisions(const ChunkExtent& chunkExtent)
{
    // Convert to TileExtent.
    TileExtent tileExtent{chunkExtent};

    return getCollisions(tileExtent);
}

void EntityLocator::removeEntity(entt::entity entity)
{
    auto entityIt{entityMap.find(entity)};
    if (entityIt != entityMap.end()) {
        // Remove the entity from each cell that it's located in.
        clearEntityLocation(entity, entityIt->second);

        // Remove the entity from the map.
        entityMap.erase(entityIt);
    }
}

std::vector<entt::entity>&
    EntityLocator::getEntitiesCoarse(const Cylinder& cylinder)
{
    // Clear the return vector.
    returnVector.clear();

    // Calc the cell extent that is intersected by the cylinder.
    CellExtent cylinderCellExtent{};
    cylinderCellExtent.x = static_cast<int>(
        std::floor((cylinder.center.x - cylinder.radius) / cellWorldWidth));
    cylinderCellExtent.y = static_cast<int>(
        std::floor((cylinder.center.y - cylinder.radius) / cellWorldWidth));
    cylinderCellExtent.xLength
        = (static_cast<int>(std::ceil((cylinder.center.x + cylinder.radius)
                                      / cellWorldWidth))
           - cylinderCellExtent.x);
    cylinderCellExtent.yLength
        = (static_cast<int>(std::ceil((cylinder.center.y + cylinder.radius)
                                      / cellWorldWidth))
           - cylinderCellExtent.y);

    // Clip the extent to the grid's bounds.
    cylinderCellExtent.intersectWith(gridCellExtent);

    // Add the entities in every intersected cell to the return vector.
    for (int x = cylinderCellExtent.x; x <= cylinderCellExtent.xMax(); ++x) {
        for (int y = cylinderCellExtent.y; y <= cylinderCellExtent.yMax();
             ++y) {
            // Add the entities in this cell to the return vector.
            std::size_t linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityVec{entityGrid[linearizedIndex]};
            returnVector.insert(returnVector.end(), entityVec.begin(),
                                entityVec.end());
        }
    }

    // Remove duplicates from the return vector.
    std::sort(returnVector.begin(), returnVector.end());
    returnVector.erase(std::unique(returnVector.begin(), returnVector.end()),
                       returnVector.end());

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getEntitiesCoarse(const BoundingBox& boundingBox)
{
    // Convert to TileExtent.
    return getEntitiesCoarse(boundingBox.asTileExtent());
}

std::vector<entt::entity>&
    EntityLocator::getEntitiesCoarse(const TileExtent& tileExtent)
{
    // Clear the return vector.
    returnVector.clear();

    // Calc the cell extent that is intersected by the tile extent.
    CellExtent tileCellExtent{tileToCellExtent(tileExtent)};

    // Clip the extent to the grid's bounds.
    tileCellExtent.intersectWith(gridCellExtent);

    // Add the entities in every intersected cell to the return vector.
    for (int x = tileCellExtent.x; x <= tileCellExtent.xMax(); ++x) {
        for (int y = tileCellExtent.y; y <= tileCellExtent.yMax(); ++y) {
            // Add the entities in this cell to the return vector.
            std::size_t linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityVec{entityGrid[linearizedIndex]};
            returnVector.insert(returnVector.end(), entityVec.begin(),
                                entityVec.end());
        }
    }

    // Remove duplicates from the return vector.
    std::sort(returnVector.begin(), returnVector.end());
    returnVector.erase(std::unique(returnVector.begin(), returnVector.end()),
                       returnVector.end());

    return returnVector;
}

std::vector<entt::entity>&
    EntityLocator::getEntitiesCoarse(const ChunkExtent& chunkExtent)
{
    // Convert to TileExtent.
    TileExtent tileExtent{chunkExtent};

    return getEntitiesCoarse(tileExtent);
}

void EntityLocator::clearEntityLocation(entt::entity entity,
                                        CellExtent& clearExtent)
{
    // Iterate through all the cells that the entity occupies.
    for (int x = clearExtent.x; x <= clearExtent.xMax(); ++x) {
        for (int y = clearExtent.y; y <= clearExtent.yMax(); ++y) {
            // Find the entity in this cell's entity vector.
            std::size_t linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityVec{entityGrid[linearizedIndex]};
            auto entityIt{
                std::find(entityVec.begin(), entityVec.end(), entity)};

            // Remove the entity from this cell's entity vector.
            if (entityIt != entityVec.end()) {
                entityVec.erase(entityIt);
            }
        }
    }
}

CellExtent EntityLocator::tileToCellExtent(const TileExtent& tileExtent)
{
    // Cast CELL_WIDTH to a float so we get float division below.
    float cellWidth{SharedConfig::CELL_WIDTH};

    CellPosition topLeft{};
    topLeft.x = static_cast<int>(std::floor(tileExtent.x / cellWidth));
    topLeft.y = static_cast<int>(std::floor(tileExtent.y / cellWidth));

    CellPosition bottomRight{};
    bottomRight.x = static_cast<int>(
        std::ceil((tileExtent.x + tileExtent.xLength) / cellWidth));
    bottomRight.y = static_cast<int>(
        std::ceil((tileExtent.y + tileExtent.yLength) / cellWidth));

    return {topLeft.x, topLeft.y, (bottomRight.x - topLeft.x),
            (bottomRight.y - topLeft.y)};
}

} // End namespace AM
