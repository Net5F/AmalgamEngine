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
        LOG_ERROR("Map length must be divisible by CELL_WIDTH");
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
    CellExtent cellExtent{};
    cellExtent.x = static_cast<int>(boundingBox.minX / cellWorldWidth);
    cellExtent.y = static_cast<int>(boundingBox.minY / cellWorldWidth);
    cellExtent.xLength = static_cast<int>(boundingBox.maxX
        - boundingBox.minX / cellWorldWidth);
    cellExtent.yLength = static_cast<int>(boundingBox.maxY
        - boundingBox.minY / cellWorldWidth);

    // If we already have a location for the entity, clear it.
    auto entityIt{entityMap.find(entity)};
    if (entityIt != entityMap.end()) {
        // Clear the entity's current location.
        clearEntityLocation(entity, entityIt->second);
    }
    else {
        // The given entity wasn't in the map, add it.
        entityMap.insert({entity, cellExtent});
    }

    // Iterate through all the cells that the entity occupies.
    int xMax{cellExtent.x + cellExtent.xLength};
    int yMax{cellExtent.y + cellExtent.yLength};
    for (int x = cellExtent.x; x < xMax; ++x) {
        for (int y = cellExtent.y; y < yMax; ++x) {
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

    // Calc a cell extent that contains the desired radius.
    CellExtent cellExtent{};
    cellExtent.x = std::floor((cylinderCenter.x - radius) / cellWorldWidth);
    cellExtent.y = std::floor((cylinderCenter.y - radius) / cellWorldWidth);
    cellExtent.xLength = std::ceil((radius * 2) / cellWorldWidth);
    cellExtent.yLength = std::ceil((radius * 2) / cellWorldWidth);

    // Iterate through all the cells in the extent.
    int xMax{cellExtent.x + cellExtent.xLength};
    int yMax{cellExtent.y + cellExtent.yLength};
    for (int x = cellExtent.x; x < xMax; ++x) {
        for (int y = cellExtent.y; y < yMax; ++x) {
            // Add the entities in this cell to the return vector.
            unsigned int linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityArr{entityGrid[linearizedIndex]};
            returnVector.insert(returnVector.end(), entityArr.begin(), entityArr.end());
        }
    }

    return returnVector;
}

std::vector<entt::entity>& EntityLocator::getEntitiesFine(const Position& cylinderCenter
        , unsigned int radius)
{
    // Run a coarse pass.
    getEntitiesCoarse(cylinderCenter, radius);

    // Iterate the results from the coarse pass, removing any entities that
    // don't actually intersect the cylinder.
    auto view{registry.view<BoundingBox>()};
    for (auto it = returnVector.begin(); it != returnVector.end();) {
        // If this entity's bounding box doesn't intersect the cylinder.
        BoundingBox& boundingBox{view.get<BoundingBox>(*it)};
        if (!(boundingBox.intersects(cylinderCenter, radius))) {
            // Swap and pop the entity ID to remove it from the return vector.
            std::iter_swap(it, returnVector.end() - 1);
            returnVector.pop_back();
        }
        else {
            // Entity is in the cylinder, proceed to the next.
            ++it;
        }
    }

    return returnVector;
}

std::vector<entt::entity>& EntityLocator::getEntitiesCoarse(const TileExtent& tileExtent)
{
    // Clear the return vector.
    returnVector.clear();

    // Calc a cell extent that contains the desired tiles.
    CellExtent cellExtent{};
    cellExtent.x = std::floor(tileExtent.x / SharedConfig::CELL_WIDTH);
    cellExtent.y = std::floor(tileExtent.y / SharedConfig::CELL_WIDTH);
    cellExtent.xLength = std::ceil(tileExtent.xLength / SharedConfig::CELL_WIDTH);
    cellExtent.yLength = std::ceil(tileExtent.yLength / SharedConfig::CELL_WIDTH);

    // Iterate through all the cells in the extent.
    int xMax{cellExtent.x + cellExtent.xLength};
    int yMax{cellExtent.y + cellExtent.yLength};
    for (int x = cellExtent.x; x < xMax; ++x) {
        for (int y = cellExtent.y; y < yMax; ++x) {
            // Add the entities in this cell to the return vector.
            unsigned int linearizedIndex{linearizeCellIndex(x, y)};
            std::vector<entt::entity>& entityArr{entityGrid[linearizedIndex]};
            returnVector.insert(returnVector.end(), entityArr.begin(), entityArr.end());
        }
    }

    return returnVector;
}

std::vector<entt::entity>& EntityLocator::getEntitiesFine(const TileExtent& tileExtent)
{
    // Run a coarse pass.
    getEntitiesCoarse(tileExtent);

    // Iterate the results from the coarse pass, removing any entities that
    // don't actually intersect the extent.
    auto view{registry.view<BoundingBox>()};
    for (auto it = returnVector.begin(); it != returnVector.end();) {
        // If this entity's bounding box doesn't intersect the extent.
        BoundingBox& boundingBox{view.get<BoundingBox>(*it)};
        if (!(boundingBox.intersects(tileExtent))) {
            // Swap and pop the entity ID to remove it from the return vector.
            std::iter_swap(it, returnVector.end() - 1);
            returnVector.pop_back();
        }
        else {
            // Entity is in the extent, proceed to the next.
            ++it;
        }
    }

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

void EntityLocator::clearEntityLocation(entt::entity entity, CellExtent& cellExtent)
{
    // Iterate through all the cells that the entity occupies.
    int xMax{cellExtent.x + cellExtent.xLength};
    int yMax{cellExtent.y + cellExtent.yLength};
    for (int x = cellExtent.x; x < xMax; ++x) {
        for (int y = cellExtent.y; y < yMax; ++x) {
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
