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
 * This locator only tracks entities, and it tracks them by their Position 
 * component. This is commonly used for networking-related queries (e.g. find all
 * entities that are within a given AoI, so we can send them network updates).
 * All entities have a Position, so they will be tracked by this locator.
 * For Collision-related queries for both entities and tile layers, see 
 * CollisionLocator.h.
 *
 * Internally, entities are organized into "cells", each of which has a size
 * corresponding to SharedConfig::ENTITY_LOCATOR_CELL_WIDTH/HEIGHT. These values
 * can be tweaked to affect performance.
 */
class EntityLocator
{
public:
    EntityLocator(entt::registry& inRegistry);

    /**
     * Sets this locator's internal grid size to match the given extent.
     */
    void setGridSize(const TileExtent& tileExtent);

    /**
     * Adds the given entity to this locator at the given position, or updates 
     * it if it's already added.
     *
     * @return true if successful, else false (position outside of locator 
     *         bounds).
     */
    bool updateEntity(entt::entity entity, const Position& position);

    /**
     * Removes the given entity from this locator, if present.
     */
    void removeEntity(entt::entity entity);

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
     * Performs a broad phase to get all entities in cells intersected by
     * the given cylinder.
     *
     * Note: All entities in the intersected cells are returned, which may
     *       include entities that aren't actually within the radius.
     */
    std::vector<entt::entity>& getEntitiesBroad(const Cylinder& cylinder);

    /**
     * Overload for BoundingBox.
     */
    std::vector<entt::entity>&
        getEntitiesBroad(const BoundingBox& boundingBox);

    /**
     * Overload for TileExtent.
     */
    std::vector<entt::entity>& getEntitiesBroad(const TileExtent& tileExtent);

    /**
     * Overload for ChunkExtent.
     */
    std::vector<entt::entity>&
        getEntitiesBroad(const ChunkExtent& chunkExtent);

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
     * Removes the given entity from the given cell.
     *
     * Note: This leaves the entity's ID in the entityMap. Only the tracked
     *       location is cleared out.
     */
    void clearEntityFromCell(entt::entity entity,
                             const CellPosition& clearPosition);

    /**
     * Returns the index in the entityGrid vector where the cell with the given
     * coordinates can be found.
     */
    inline std::size_t
        linearizeCellIndex(const CellPosition& cellPosition) const
    {
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

    /** Used for fetching entity positions during narrow phases. */
    entt::registry& registry;

    /** The grid's extent, with cells as the unit. */
    CellExtent gridCellExtent;

    /** The outer vector is a 3D grid stored in row-major order, holding the
        grid's cells.
        Each element in the grid is a vector of entities--the entities that
        currently intersect with that cell. */
    std::vector<std::vector<entt::entity>> entityGrid;

    /** A map of entity ID -> the cell that the entity is located in.
        Used to easily find the entity during removal. */
    std::unordered_map<entt::entity, CellPosition> entityMap;

    /** The vector that we use to return results. */
    std::vector<entt::entity> returnVector;
};

} // End namespace AM
