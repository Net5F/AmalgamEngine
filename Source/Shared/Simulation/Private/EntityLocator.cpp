#include "EntityLocator.h"
#include "SharedConfig.h"
#include "Position.h"
#include "BoundingBox.h"
#include "Log.h"
#include "entt/entity/registry.hpp"
#include <cmath>
#include <algorithm>

namespace AM
{

EntityLocator::EntityLocator(entt::registry& inRegistry)
: registry{inRegistry}
, cellExtent{}
, cellWorldWidth{SharedConfig::CELL_WIDTH * SharedConfig::TILE_WORLD_WIDTH}
{
}

void EntityLocator::setGridSize(unsigned int inMapXLengthTiles,
                                unsigned int inMapYLengthTiles)
{
    if (((inMapXLengthTiles % SharedConfig::CELL_WIDTH) != 0)
        || ((inMapYLengthTiles % SharedConfig::CELL_WIDTH) != 0)) {
        LOG_FATAL("Map length must be divisible by CELL_WIDTH");
    }

    // Set our grid size to match the tile map.
    cellExtent.xLength = (inMapXLengthTiles / SharedConfig::CELL_WIDTH);
    cellExtent.yLength = (inMapYLengthTiles / SharedConfig::CELL_WIDTH);

    // Resize the grid to fit the map.
    entityGrid.resize(cellExtent.xLength * cellExtent.yLength);
}

void EntityLocator::setEntityLocation(entt::entity entity, const BoundingBox& boundingBox)
{
    // Find the cells that the bounding box intersects.
    CellExtent boxCellExtent{};
    boxCellExtent.x = std::floor(boundingBox.minX / cellWorldWidth);
    boxCellExtent.y = std::floor(boundingBox.minY / cellWorldWidth);
    boxCellExtent.xLength = (std::ceil(boundingBox.maxX / cellWorldWidth)
        - boxCellExtent.x);
    boxCellExtent.yLength = (std::ceil(boundingBox.maxY / cellWorldWidth)
        - boxCellExtent.y);

    // Clip the extent to the grid's bounds.
    boxCellExtent.intersectWith(cellExtent);

    // If we already have a location for the entity, clear it.
    auto entityIt{entityMap.find(entity)};
    if (entityIt != entityMap.end()) {
        // Clear the entity's current location.
        clearEntityLocation(entity, entityIt->second);
    }

    // Add the entity to the map, or update its extent if it already exists.
    entityMap.insert_or_assign(entity, boxCellExtent);

    // Add the entity to all the cells that it occupies.
    int xMax{boxCellExtent.x + boxCellExtent.xLength};
    int yMax{boxCellExtent.y + boxCellExtent.yLength};
    for (int x = boxCellExtent.x; x < xMax; ++x) {
        for (int y = boxCellExtent.y; y < yMax; ++y) {
            // Add the entity to this cell's entity array.
            unsigned int linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityArr{entityGrid[linearizedIndex]};

            entityArr.push_back(entity);
        }
    }
}

std::vector<entt::entity>& EntityLocator::getEntitiesCoarse(const Position& cylinderCenter
        , unsigned int radius)
{
    // Clear the return vector.
    returnVector.clear();

    // Calc the cell extent that is intersected by the cylinder.
    CellExtent cylinderCellExtent{};
    cylinderCellExtent.x = std::floor((cylinderCenter.x - radius) / cellWorldWidth);
    cylinderCellExtent.y = std::floor((cylinderCenter.y - radius) / cellWorldWidth);
    cylinderCellExtent.xLength = (std::ceil((cylinderCenter.x + radius) / cellWorldWidth)
        - cylinderCellExtent.x);
    cylinderCellExtent.yLength = (std::ceil((cylinderCenter.y + radius) / cellWorldWidth)
        - cylinderCellExtent.y);

    // Clip the extent to the grid's bounds.
    cylinderCellExtent.intersectWith(cellExtent);

    // Add the entities in every intersected cell to the return vector.
    int xMax{cylinderCellExtent.x + cylinderCellExtent.xLength};
    int yMax{cylinderCellExtent.y + cylinderCellExtent.yLength};
    for (int x = cylinderCellExtent.x; x < xMax; ++x) {
        for (int y = cylinderCellExtent.y; y < yMax; ++y) {
            // Add the entities in this cell to the return vector.
            unsigned int linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityArr{entityGrid[linearizedIndex]};
            returnVector.insert(returnVector.end(), entityArr.begin(), entityArr.end());
        }
    }

    // Remove duplicates from the return vector.
    std::sort(returnVector.begin(), returnVector.end());
    returnVector.erase(std::unique(returnVector.begin(), returnVector.end()),
        returnVector.end());

    return returnVector;
}

std::vector<entt::entity>& EntityLocator::getEntitiesFine(const Position& cylinderCenter
        , unsigned int radius)
{
    // Run a coarse pass.
    getEntitiesCoarse(cylinderCenter, radius);

    // Erase any entities that don't actually intersect the cylinder.
    auto view{registry.view<BoundingBox>()};
    std::erase_if(returnVector, [&view, &cylinderCenter, radius](entt::entity entity) {
        BoundingBox& boundingBox{view.get<BoundingBox>(entity)};
        return !(boundingBox.intersects(cylinderCenter, radius));
    });

    return returnVector;
}

std::vector<entt::entity>& EntityLocator::getEntitiesCoarse(const TileExtent& tileExtent)
{
    // Clear the return vector.
    returnVector.clear();

    // Calc the cell extent that is intersected by the tile extent.
    CellExtent tileCellExtent{};
    tileCellExtent.x = std::floor(tileExtent.x / SharedConfig::CELL_WIDTH);
    tileCellExtent.y = std::floor(tileExtent.y / SharedConfig::CELL_WIDTH);
    tileCellExtent.xLength = std::ceil(tileExtent.xLength / SharedConfig::CELL_WIDTH);
    tileCellExtent.yLength = std::ceil(tileExtent.yLength / SharedConfig::CELL_WIDTH);

    // Clip the extent to the grid's bounds.
    tileCellExtent.intersectWith(cellExtent);

    // Add the entities in every intersected cell to the return vector.
    int xMax{tileCellExtent.x + tileCellExtent.xLength};
    int yMax{tileCellExtent.y + tileCellExtent.yLength};
    for (int x = tileCellExtent.x; x < xMax; ++x) {
        for (int y = tileCellExtent.y; y < yMax; ++y) {
            // Add the entities in this cell to the return vector.
            unsigned int linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityArr{entityGrid[linearizedIndex]};
            returnVector.insert(returnVector.end(), entityArr.begin(), entityArr.end());
        }
    }

    // Remove duplicates from the return vector.
    std::sort(returnVector.begin(), returnVector.end());
    returnVector.erase(std::unique(returnVector.begin(), returnVector.end()),
        returnVector.end());

    return returnVector;
}

std::vector<entt::entity>& EntityLocator::getEntitiesFine(const TileExtent& tileExtent)
{
    // Run a coarse pass.
    getEntitiesCoarse(tileExtent);

    // Erase any entities that don't actually intersect the extent.
    auto view{registry.view<BoundingBox>()};
    std::erase_if(returnVector, [&view, &tileExtent](entt::entity entity) {
        BoundingBox& boundingBox{view.get<BoundingBox>(entity)};
        return !(boundingBox.intersects(tileExtent));
    });

    return returnVector;
}

void EntityLocator::removeEntity(entt::entity entity)
{
    auto entityIt{entityMap.find(entity)};
    if (entityIt != entityMap.end()) {
        // Remove the entity from each cell that it's located in.
        clearEntityLocation(entity, entityIt->second);
    }
}

void EntityLocator::clearEntityLocation(entt::entity entity, CellExtent& clearExtent)
{
    // Iterate through all the cells that the entity occupies.
    int xMax{clearExtent.x + clearExtent.xLength};
    int yMax{clearExtent.y + clearExtent.yLength};
    for (int x = clearExtent.x; x < xMax; ++x) {
        for (int y = clearExtent.y; y < yMax; ++y) {
            // Find the entity in this cell's entity array.
            unsigned int linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityArr{entityGrid[linearizedIndex]};
            auto entityIt{std::find(entityArr.begin(), entityArr.end(), entity)};

            // Remove the entity from this cell's entity array.
            if (entityIt != entityArr.end()) {
                entityArr.erase(entityIt);
            }
        }
    }
}

} // End namespace AM
