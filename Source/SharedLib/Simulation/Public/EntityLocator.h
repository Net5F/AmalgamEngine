#pragma once

#include "CellExtent.h"
#include "CellPosition.h"
#include "TileExtent.h"
#include "ChunkExtent.h"
#include "entt/fwd.hpp"
#include <vector>
#include <unordered_map>

namespace AM
{
struct Position;
struct Cylinder;
struct BoundingBox;

/**
 * A spatial partitioning grid that tracks where entities are located.
 *
 * Used to quickly determine which entities are located within a given extent
 * of the world.
 *
 * Internally, entities are organized into "cells", each of which has a size
 * corresponding to SharedConfig::CELL_WIDTH. This value can be tweaked to
 * affect performance.
 */
class EntityLocator
{
public:
    EntityLocator(entt::registry& inRegistry);

    /**
     * Sets the size of the entity grid to match the given extent and resizes 
     * the entityGrid vector.
     */
    void setGridSize(const TileExtent& tileExtent);

    /**
     * Sets the given entity's location to the location of the given bounding
     * box.
     *
     * Note: Assumes all values are valid. Don't pass in values that are
     *       outside of the map bounds.
     *
     * @param entity  The entity to set the location of.
     * @param boundingBox  The entity's bounding box.
     */
    void setEntityLocation(entt::entity entity, const BoundingBox& boundingBox);

    /**
     * Returns all entities whose positions intersect the given cylinder.
     *
     * Note: Because this uses position, it exhibits the commutative property
     *       (if a cylinder centered on entityA returns entityB, the reverse
     *       will also be true).
     */
    std::vector<entt::entity>& getEntities(const Cylinder& cylinder);

    /**
     * Overload for TileExtent.
     */
    std::vector<entt::entity>& getEntities(const TileExtent& tileExtent);

    /**
     * Overload for ChunkExtent.
     */
    std::vector<entt::entity>& getEntities(const ChunkExtent& chunkExtent);

    /**
     * Returns all entities whose collision boxes intersect the given cylinder.
     *
     * Note: Because this uses collision and collision boxes vary in size and
     *       position, it does not exhibit the commutative property (if a
     *       cylinder centered on entityA returns entityB, the reverse may not
     *       be true).
     */
    std::vector<entt::entity>& getCollisions(const Cylinder& cylinder);

    /**
     * Overload for BoundingBox.
     */
    std::vector<entt::entity>& getCollisions(const BoundingBox& boundingBox);

    /**
     * Overload for TileExtent.
     */
    std::vector<entt::entity>& getCollisions(const TileExtent& tileExtent);

    /**
     * Overload for ChunkExtent.
     */
    std::vector<entt::entity>& getCollisions(const ChunkExtent& chunkExtent);

    /**
     * If we're tracking the given entity, removes it from the entityGrid and
     * entityMap.
     */
    void removeEntity(entt::entity entity);

private:
    /** The width of a grid cell in world units. */
    static constexpr float CELL_WORLD_WIDTH{
        SharedConfig::ENTITY_LOCATOR_CELL_WIDTH
        * SharedConfig::TILE_WORLD_WIDTH};

    /** The height of a grid cell in world units. */
    static constexpr float CELL_WORLD_HEIGHT{
        SharedConfig::ENTITY_LOCATOR_CELL_HEIGHT
        * SharedConfig::TILE_WORLD_HEIGHT};

    /**
     * Performs a coarse pass to get all entities in cells intersected by
     * the given cylinder.
     *
     * Note: All entities in the intersected cells are returned, which may
     *       include entities that aren't actually within the radius.
     */
    std::vector<entt::entity>& getEntitiesCoarse(const Cylinder& cylinder);

    /**
     * Overload for BoundingBox.
     */
    std::vector<entt::entity>&
        getEntitiesCoarse(const BoundingBox& boundingBox);

    /**
     * Overload for TileExtent.
     */
    std::vector<entt::entity>& getEntitiesCoarse(const TileExtent& tileExtent);

    /**
     * Overload for ChunkExtent.
     */
    std::vector<entt::entity>&
        getEntitiesCoarse(const ChunkExtent& chunkExtent);

    /**
     * Removes the given entity from the cells within the given extent.
     *
     * Note: This leaves the entity's ID in the entityMap. Only the tracked
     *       location is cleared out.
     */
    void clearEntityLocation(entt::entity entity, CellExtent& clearExtent);

    /**
     * Returns the index in the entityGrid vector where the cell with the given
     * coordinates can be found.
     */
    inline std::size_t
        linearizeCellIndex(const CellPosition& cellPosition) const
    {
        // TODO: Switch to 3d morton code
        // Translate the given position from actual-space to positive-space.
        CellPosition positivePosition{cellPosition.x - gridCellExtent.x,
                                      cellPosition.y - gridCellExtent.y,
                                      cellPosition.z - gridCellExtent.z};

        return static_cast<std::size_t>(
            (gridCellExtent.xLength * gridCellExtent.yLength
             * positivePosition.z)
            + (gridCellExtent.xLength * positivePosition.y)
            + positivePosition.x);
    }

    /**
     * Converts the given tile extent to a cell extent.
     */
    CellExtent tileToCellExtent(const TileExtent& tileExtent);

    /** Used for fetching entity bounding boxes while doing a fine pass. */
    entt::registry& registry;

    /** The grid's extent, with cells as the unit. */
    CellExtent gridCellExtent;

    /** The outer vector is a 3D grid stored in row-major order, holding the
        grid's cells.
        Each element in the grid is a vector of entities--the entities that
        currently intersect with that cell. */
    std::vector<std::vector<entt::entity>> entityGrid;

    /** A map of entity ID -> the cells that the entity is located in.
        Used to easily clear out old entity data before setting their new
        location. */
    std::unordered_map<entt::entity, CellExtent> entityMap;

    /** The vector that we use to return results. */
    std::vector<entt::entity> returnVector;
};

} // End namespace AM
