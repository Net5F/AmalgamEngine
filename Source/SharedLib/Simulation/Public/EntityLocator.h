#pragma once

#include "CellExtent.h"
#include "TileExtent.h"
#include "ChunkExtent.h"
#include "entt/fwd.hpp"
#include <vector>
#include <unordered_map>

namespace AM
{
struct Position;
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
     * Sets the size of the entity grid and resizes the entityGrid vector.
     *
     * @param inMapXLengthTiles  The X length of the tile map, in tiles.
     * @param inMapYLengthTiles  The Y length of the tile map, in tiles.
     */
    void setGridSize(unsigned int inMapXLengthTiles,
                     unsigned int inMapYLengthTiles);

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
     * Performs a coarse pass to get all entities in cells intersected by
     * the given cylinder.
     *
     * Note: All entities in the intersected cells are returned, which may
     *       include entities that aren't actually within the radius.
     *
     * @param cylinderCenter  The position to cast the radius from.
     * @param radius  The length of the radius to cast.
     */
    std::vector<entt::entity>& getEntitiesCoarse(const Position& cylinderCenter,
                                                 unsigned int radius);

    /**
     * Performs a fine pass to return all entities that intersect the given
     * cylinder.
     *
     * @param cylinderCenter  The position to cast the radius from.
     * @param radius  The length of the radius to cast.
     */
    std::vector<entt::entity>& getEntitiesFine(const Position& cylinderCenter,
                                               unsigned int radius);

    /**
     * Overload for TileExtent.
     */
    std::vector<entt::entity>& getEntitiesCoarse(const TileExtent& tileExtent);

    /**
     * Overload for TileExtent.
     */
    std::vector<entt::entity>& getEntitiesFine(const TileExtent& tileExtent);

    /**
     * Overload for ChunkExtent.
     */
    std::vector<entt::entity>&
        getEntitiesCoarse(const ChunkExtent& chunkExtent);

    /**
     * Overload for ChunkExtent.
     */
    std::vector<entt::entity>& getEntitiesFine(const ChunkExtent& chunkExtent);

    /**
     * If we're tracking the given entity, removes it from the entityGrid and
     * entityMap.
     */
    void removeEntity(entt::entity entity);

private:
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
    inline unsigned int linearizeCellIndex(int x, int y) const
    {
        return (y * cellExtent.xLength) + x;
    }

    /**
     * Converts the given tile extent to a cell extent.
     */
    CellExtent tileToCellExtent(const TileExtent& tileExtent);

    /** Used for fetching entity bounding boxes while doing a fine pass. */
    entt::registry& registry;

    /** The grid's extent, with cells as the unit. */
    CellExtent cellExtent;

    /** The width of a grid cell in world units. */
    const float cellWorldWidth;

    /** The outer vector is a 2D grid stored in row-major order, holding the
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
