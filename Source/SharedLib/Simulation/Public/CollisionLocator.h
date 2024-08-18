#pragma once

#include "CellExtent.h"
#include "CellPosition.h"
#include "TilePosition.h"
#include "TileExtent.h"
#include "ChunkExtent.h"
#include "Terrain.h"
#include "SharedConfig.h"
#include "entt/fwd.hpp"
#include <vector>

namespace AM
{
class Tile;
struct Cylinder;
struct BoundingBox;
struct MinMaxBox;

/**
 * The types of world objects that can be collided with.
 * Note: If desired, we can switch this to use our engine/project enum 
 *       pattern and let the project add custom types.
 */
struct CollisionObjectType {
    enum Value : Uint8
    {
        ClientEntity = 0b00000001,
        NonClientEntity = 0b00000010,
        TileLayer = 0b00000100,
    };
};

/** A bitmask made from CollisionObjectType values. */
using CollisionObjectTypeMask = Uint16;

/**
 * A spatial partitioning grid that tracks where collision volumes are located.
 *
 * Used to quickly determine which collision volumes are located within a given 
 * extent of the world.
 *
 * This locator tracks both entities and tile layers, and it tracks them by 
 * their bounding volume (i.e. collision). Not all entities and tile layers 
 * have collision, so they may not be tracked by this locator.
 * For Position-related entity queries, see EntityLocator.h.
 *
 * Internally, collision volumes are organized into "cells", each of which has 
 * a size corresponding to SharedConfig::COLLISION_LOCATOR_CELL_WIDTH. These 
 * values can be tweaked to affect performance.
 */
class CollisionLocator
{
public:
    CollisionLocator();

    /**
     * Sets this locator's internal grid size to match the given extent.
     */
    void setGridSize(const TileExtent& tileExtent);

    /**
     * Adds the given entity to this locator, or updates it if it's already 
     * added.
     *
     * @param entity The entity to add.
     * @param collisionVolume The entity's collision volume.
     * @param objectType Must be one of the entity types.
     */
    void updateEntity(entt::entity entity, const BoundingBox& collisionVolume,
                      CollisionObjectType::Value objectType);

    /**
     * Adds the given tile to this locator, or updates it if it's already added.
     * 
     * @param tilePosition The tile's position.
     * @param tile The tile to add.
     */
    void updateTile(const TilePosition& tilePosition, const Tile& tile);

    /**
     * Removes the given entity from this locator, if present.
     */
    void removeEntity(entt::entity entity);

    /**
     * A world object's collision information.
     */
    struct CollisionInfo
    {
        /** The world object's collision volume. */
        BoundingBox collisionVolume{};

        /** The type of world object that this volume belongs to. */
        CollisionObjectType::Value objectType{};

        /** If objectType is one of the entity types, this is the entity's 
            ID. */
        entt::entity entity{};
    };

    /**
     * Returns all collision volumes that intersect the given cylinder.
     *
     * Note: Because collision boxes vary in size, results are not commutative
     *       (if a cylinder centered on volumeA intersects volumeB, the reverse
     *       may not be true).
     *
     * @param objectTypeMask The bitmask to use when filtering objects. If an 
     *                       object type is present in the mask, it will be 
     *                       included in the results.
     * @return Pointers to the info of each hit world object. These pointers 
     *         are not stable, and may become invalid when any of this locator's 
     *         functions are called.
     */
    std::vector<const CollisionInfo*>&
        getCollisions(const Cylinder& cylinder,
                      CollisionObjectTypeMask objectTypeMask);

    /**
     * Overload for BoundingBox.
     */
    std::vector<const CollisionInfo*>&
        getCollisions(const BoundingBox& boundingBox,
                      CollisionObjectTypeMask objectTypeMask);

    /**
     * Overload for TileExtent.
     */
    std::vector<const CollisionInfo*>&
        getCollisions(const TileExtent& tileExtent,
                      CollisionObjectTypeMask objectTypeMask);

    /**
     * Overload for ChunkExtent.
     */
    std::vector<const CollisionInfo*>&
        getCollisions(const ChunkExtent& chunkExtent,
                      CollisionObjectTypeMask objectTypeMask);

    /**
     * Performs a broad phase to get all collision volumes in cells intersected
     * by the given cylinder.
     *
     * Note: All volumes in the intersected cells are returned, which may
     *       include volumes that aren't actually within the radius.
     *
     * @param objectTypeMask The bitmask to use when filtering objects. If an
     *                       object type is present in the mask, it will be
     *                       included in the results.
     */
    std::vector<const CollisionInfo*>&
        getCollisionsBroad(const Cylinder& cylinder,
                           CollisionObjectTypeMask objectTypeMask);

    /**
     * Overload for BoundingBox.
     */
    std::vector<const CollisionInfo*>&
        getCollisionsBroad(const BoundingBox& boundingBox,
                           CollisionObjectTypeMask objectTypeMask);

    /**
     * Overload for MinMaxBox.
     */
    std::vector<const CollisionInfo*>&
        getCollisionsBroad(const MinMaxBox& boundingBox,
                           CollisionObjectTypeMask objectTypeMask);

    /**
     * Overload for TileExtent.
     */
    std::vector<const CollisionInfo*>&
        getCollisionsBroad(const TileExtent& tileExtent,
                           CollisionObjectTypeMask objectTypeMask);

    /**
     * Overload for ChunkExtent.
     */
    std::vector<const CollisionInfo*>&
        getCollisionsBroad(const ChunkExtent& chunkExtent,
                           CollisionObjectTypeMask objectTypeMask);

private:
    /** The width of a grid cell in world units. */
    static constexpr float CELL_WORLD_WIDTH{
        SharedConfig::COLLISION_LOCATOR_CELL_WIDTH
        * SharedConfig::TILE_WORLD_WIDTH};

    /** The height of a grid cell in world units. */
    static constexpr float CELL_WORLD_HEIGHT{
        SharedConfig::COLLISION_LOCATOR_CELL_HEIGHT
        * SharedConfig::TILE_WORLD_HEIGHT};

    /** A value to use in terrainGrid to indicate that a tile has no terrain. */
    static constexpr Terrain::Value EMPTY_TERRAIN{SDL_MAX_UINT8};

    /**
     * Adds the given index to the collisionGrid cells within the given extent.
     */
    void addCollisionVolumeToCells(Uint16 volumeIndex,
                                   const CellExtent& cellExtent);

    /**
     * Removes the given index from the collisionGrid cells within the given 
     * extent.
     */
    void clearCollisionVolumeFromCells(Uint16 volumeIndex,
                                       const CellExtent& clearExtent);

    /**
     * Adds the given tile's collision volumes to the collision and terrain 
     * grids.
     */
    void addTileCollisionVolumes(const TilePosition& tilePosition,
                                 const Tile& tile);

    /**
     * Performs a broad phase to get all collision volumes in cells intersected
     * by the given cylinder.
     *
     * Note: All volumes in the intersected cells are returned, which may
     *       include volumes that aren't actually within the radius.
     *
     * @param objectTypeMask The bitmask to use when filtering objects. If an
     *                       object type is present in the mask, it will be
     *                       included in the results.
     *
     * @pre tileExtent and cellExtent must be pre-clipped to this locator's 
     *      bounds.
     */
    std::vector<const CollisionInfo*>&
        getCollisionsBroad(const TileExtent& tileExtent,
                           const CellExtent& cellExtent,
                           CollisionObjectTypeMask objectTypeMask);

    /**
     * Returns the index in the collisionGrid vector where the cell with the 
     * given coordinates can be found.
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

    /**
     * Returns the index in the terrainGrid vector where the tile with the 
     * given coordinates can be found.
     */
    inline std::size_t
        linearizeTileIndex(const TilePosition& tilePosition) const
    {
        // Translate the given position from actual-space to positive-space.
        TilePosition positivePosition{tilePosition.x - gridTileExtent.x,
                                      tilePosition.y - gridTileExtent.y,
                                      tilePosition.z - gridTileExtent.z};

        return static_cast<std::size_t>(
            (gridTileExtent.xLength * gridTileExtent.yLength
             * positivePosition.z)
            + (gridTileExtent.xLength * positivePosition.y)
            + positivePosition.x);
    }

    /** The grid's extent, with tiles as the unit. */
    TileExtent gridTileExtent;

    /** The grid's extent, with cells as the unit. */
    CellExtent gridCellExtent;

    /** The collision volume and related info for each world object that this 
        locator is tracking. */
    std::vector<CollisionInfo> collisionVolumes;

    /** Tracks which indices in collisionVolumes are free to use. */
    std::vector<Uint16> freeCollisionVolumesIndices;

    /** The outer vector is a 3D grid stored in row-major order, holding the
        grid's cells.
        Each element in the grid is a vector of volumes--the volumes that
        currently intersect with that cell (represented by their index in 
        collisionVolumes). */
    std::vector<std::vector<Uint16>> collisionGrid;

    /** A map of entities -> the index of their collision volumes in 
        collisionVolumes. */
    std::unordered_map<entt::entity, Uint16> entityMap;
    /** A map of tiles -> the indices of their layer's collision volumes in 
        collisionVolumes. */
    std::unordered_map<TilePosition, std::vector<Uint16>> tileMap;

    /** A 3D grid where each element holds the terrain of the associated tile.
        Since terrain can be fully described by its 1B value, it's more 
        efficient to store the value and construct the bounding box as needed 
        instead of storing it in collisionGrid. */
    std::vector<Terrain::Value> terrainGrid;

    /** Holds the collision info of any Terrain tile layers that were hit during
        the last query (so that the query result has somewhere to point to). */
    std::vector<CollisionInfo> terrainCollisionVolumes;

    /** A scratch vector used for gathering results during the broad phase. */
    std::vector<Uint16> indexVector;

    /** The vector that we use to return results. */
    std::vector<const CollisionInfo*> returnVector;
};

} // End namespace AM
