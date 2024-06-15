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
{
}

void EntityLocator::setGridSize(const TileExtent& mapTileExtent)
{
    // Cast constants so we get appropriate division below.
    static constexpr int CELL_WIDTH{
        static_cast<int>(SharedConfig::ENTITY_LOCATOR_CELL_WIDTH)};
    static constexpr int CELL_HEIGHT{
        static_cast<int>(SharedConfig::ENTITY_LOCATOR_CELL_HEIGHT)};
    static constexpr float CELL_WIDTH_F{static_cast<float>(CELL_WIDTH)};
    static constexpr float CELL_HEIGHT_F{static_cast<float>(CELL_HEIGHT)};

    // Set our grid size to match the tile map.
    gridCellExtent.x = mapTileExtent.x / CELL_WIDTH;
    gridCellExtent.y = mapTileExtent.y / CELL_WIDTH;
    gridCellExtent.z = mapTileExtent.z / CELL_HEIGHT;
    gridCellExtent.xLength
        = static_cast<int>(std::ceil(mapTileExtent.xLength / CELL_WIDTH_F));
    gridCellExtent.yLength
        = static_cast<int>(std::ceil(mapTileExtent.yLength / CELL_WIDTH_F));
    gridCellExtent.zLength
        = static_cast<int>(std::ceil(mapTileExtent.zLength / CELL_HEIGHT_F));

    // Resize the grid to fit the map.
    entityGrid.resize(gridCellExtent.xLength * gridCellExtent.yLength
                      * gridCellExtent.zLength);
}

void EntityLocator::setEntityLocation(entt::entity entity,
                                      const BoundingBox& boundingBox)
{
    // Find the cells that the bounding box intersects.
    CellExtent boxCellExtent{};
    boxCellExtent.x
        = static_cast<int>(std::floor(boundingBox.minX / CELL_WORLD_WIDTH));
    boxCellExtent.y
        = static_cast<int>(std::floor(boundingBox.minY / CELL_WORLD_WIDTH));
    boxCellExtent.z
        = static_cast<int>(std::floor(boundingBox.minZ / CELL_WORLD_HEIGHT));
    boxCellExtent.xLength
        = (static_cast<int>(std::ceil(boundingBox.maxX / CELL_WORLD_WIDTH))
           - boxCellExtent.x);
    boxCellExtent.yLength
        = (static_cast<int>(std::ceil(boundingBox.maxY / CELL_WORLD_WIDTH))
           - boxCellExtent.y);
    boxCellExtent.zLength
        = (static_cast<int>(std::ceil(boundingBox.maxZ / CELL_WORLD_HEIGHT))
           - boxCellExtent.z);

    if (!(gridCellExtent.containsExtent(boxCellExtent))) {
        LOG_ERROR("Tried to track entity that is outside of the locator's "
                  "grid: (%d, %d, %d, %d, %d, %d)ce.",
                  boxCellExtent.x, boxCellExtent.y, boxCellExtent.z,
                  boxCellExtent.xLength, boxCellExtent.yLength,
                  boxCellExtent.zLength);
        return;
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
    for (int z{boxCellExtent.z}; z <= boxCellExtent.zMax(); ++z) {
        for (int y{boxCellExtent.y}; y <= boxCellExtent.yMax(); ++y) {
            for (int x{boxCellExtent.x}; x <= boxCellExtent.xMax(); ++x) {
                // Add the entity to this cell's entity vector.
                std::size_t linearizedIndex{linearizeCellIndex({x, y, z})};
                std::vector<entt::entity>& entityVec{
                    entityGrid[linearizedIndex]};

                entityVec.push_back(entity);
            }
        }
    }
}

std::vector<entt::entity>& EntityLocator::getEntities(const Cylinder& cylinder)
{
    AM_ASSERT(cylinder.radius >= 0, "Cylinder can't have negative radius.");

    // Run a coarse pass.
    getEntitiesCoarse(cylinder);

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
    std::erase_if(returnVector, [this, &cylinder](entt::entity entity) {
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
        std::floor((cylinder.center.x - cylinder.radius) / CELL_WORLD_WIDTH));
    cylinderCellExtent.y = static_cast<int>(
        std::floor((cylinder.center.y - cylinder.radius) / CELL_WORLD_WIDTH));
    // TODO: This is incorrect
    cylinderCellExtent.z = static_cast<int>(
        std::floor((cylinder.center.z - cylinder.radius) / CELL_WORLD_HEIGHT));
    cylinderCellExtent.xLength
        = (static_cast<int>(std::ceil((cylinder.center.x + cylinder.radius)
                                      / CELL_WORLD_WIDTH))
           - cylinderCellExtent.x);
    cylinderCellExtent.yLength
        = (static_cast<int>(std::ceil((cylinder.center.y + cylinder.radius)
                                      / CELL_WORLD_WIDTH))
           - cylinderCellExtent.y);
    cylinderCellExtent.zLength
        = (static_cast<int>(std::ceil((cylinder.center.z + cylinder.radius)
                                      / CELL_WORLD_HEIGHT))
           - cylinderCellExtent.z);

    // Clip the extent to the grid's bounds.
    cylinderCellExtent.intersectWith(gridCellExtent);

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
    for (int z{clearExtent.z}; z <= clearExtent.zMax(); ++z) {
        for (int y{clearExtent.y}; y <= clearExtent.yMax(); ++y) {
            for (int x{clearExtent.x}; x <= clearExtent.xMax(); ++x) {
                // Find the entity in this cell's entity vector.
                std::size_t linearizedIndex{linearizeCellIndex({x, y, z})};
                std::vector<entt::entity>& entityVec{
                    entityGrid[linearizedIndex]};
                auto entityIt{
                    std::find(entityVec.begin(), entityVec.end(), entity)};

                // Remove the entity from this cell's entity vector.
                if (entityIt != entityVec.end()) {
                    entityVec.erase(entityIt);
                }
            }
        }
    }
}

CellExtent EntityLocator::tileToCellExtent(const TileExtent& tileExtent)
{
    // Cast constants to float so we get float division below.
    static constexpr float CELL_WIDTH{
        static_cast<float>(SharedConfig::ENTITY_LOCATOR_CELL_WIDTH)};
    static constexpr float CELL_HEIGHT{
        static_cast<float>(SharedConfig::ENTITY_LOCATOR_CELL_HEIGHT)};

    CellPosition origin{};
    origin.x = static_cast<int>(std::floor(tileExtent.x / CELL_WIDTH));
    origin.y = static_cast<int>(std::floor(tileExtent.y / CELL_WIDTH));
    origin.z = static_cast<int>(std::floor(tileExtent.z / CELL_HEIGHT));

    CellPosition extent{};
    extent.x = static_cast<int>(
        std::ceil((tileExtent.x + tileExtent.xLength) / CELL_WIDTH));
    extent.y = static_cast<int>(
        std::ceil((tileExtent.y + tileExtent.yLength) / CELL_WIDTH));
    extent.z = static_cast<int>(
        std::ceil((tileExtent.z + tileExtent.zLength) / CELL_HEIGHT));

    return {origin.x,
            origin.y,
            origin.z,
            (extent.x - origin.x),
            (extent.y - origin.y),
            (extent.z - origin.z)};
}

} // End namespace AM
