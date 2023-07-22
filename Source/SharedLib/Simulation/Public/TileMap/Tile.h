#pragma once

#include "BoundingBox.h"
#include "TileLayers.h"
#include <vector>
#include <span>
#include <array>
#include <memory>

namespace AM
{
struct Sprite;

/**
 * A 32x32-unit tile in the tile map.
 *
 * A tile consists of layers of sprites, which can be floors, walls, etc.
 *
 * Tiles contain no logic. If something on a tile requires logic, e.g. a tree
 * growing over time, it must have a system act upon it.
 */
class Tile {
public:
    Tile();

    /**
     * Returns the collision boxes of each of this tile's layers.
     * Note: The returned vector may be empty, if this tile has no walls or 
     *       objects.
     */
    const std::vector<BoundingBox>& getCollisionBoxes() const;

    /**
     * @return This tile's floor layer.
     */
    const FloorTileLayer& getFloor() const;
    FloorTileLayer& getFloor();

    /**
     * @return This tile's floor covering layers.
     */
    const std::vector<FloorCoveringTileLayer>& getFloorCoverings() const;
    std::vector<FloorCoveringTileLayer>& getFloorCoverings();

    /**
     * @return This tile's wall layers.
     */
    const std::array<WallTileLayer, 2>& getWalls() const;
    std::array<WallTileLayer, 2>& getWalls();

    /**
     * @return This tile's object layers.
     */
    const std::vector<ObjectTileLayer>& getObjects() const;
    std::vector<ObjectTileLayer>& getObjects();

    /**
     * @return true if this tile has a West wall.
     */
    bool hasWestWall() const;

    /**
     * @return true if this tile has a North wall.
     */
    bool hasNorthWall() const;

    /**
     * Clears the collisionBoxes vector, then refills it with all of this 
     * tile's walls and objects.
     * 
     * @param tileX  This tile's X-axis tile coordinate.
     * @param tileY  This tile's Y-axis tile coordinate.
     */
    void rebuildCollision(int tileX, int tileY);

private:
    // TODO: Maybe eventually switch to an alternative vector type that 
    //       has a smaller footprint but only supports forward iterators.
    /** Holds this tile's collision boxes.
        We pre-calculate these and store them contiguously to speed up collision
        checking. */
    std::vector<BoundingBox> collisionBoxes;

    struct Layers
    {
        /** Tiles can only have 1 floor.
            If the floor is cleared, its sprite set will be nullptr. */
        FloorTileLayer floor{};

        /** Tiles can have any number of floor coverings. */
        std::vector<FloorCoveringTileLayer> floorCoverings{};

        /** Tiles can have up to 2 walls.
            If a wall doesn't exist, its sprite set will be nullptr and its
            WallType will be None.
            Possible states: [None, None], [West, None], [None, North],
                             [West, NEGapFill], [None, NWGapFill] */
        std::array<WallTileLayer, 2> walls{};

        /** Tiles can have any number of objects. */
        std::vector<ObjectTileLayer> objects{};
    };

    /** The sprite layers that are on this tile.  */
    std::unique_ptr<Layers> layers;

    /**
     * Returns the given sprite's modelBounds, translated to world space and 
     * offset to the given tile coords.
     */
    BoundingBox calcWorldBoundsForSprite(int tileX, int tileY,
                                         const Sprite* sprite);
};

} // End namespace AM
