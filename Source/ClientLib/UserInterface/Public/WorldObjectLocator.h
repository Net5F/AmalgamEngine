#pragma once

#include "Camera.h"
#include "WorldObjectID.h"
#include "CellExtent.h"
#include "CellPosition.h"
#include "Config.h"
#include <SDL_rect.h>
#include <vector>
#include <unordered_map>

namespace AM
{
namespace Client
{

/**
 * A spatial partitioning hash map that tracks where world objects (tile
 * layers, entities) are located and the order in which they were drawn to the
 * screen.
 *
 * Used by the UI to quickly find which tile layer or entity the mouse is
 * hovering over.
 *
 * In constract to the similar EntityLocator and CollisionLocator, this class 
 * is UI-centric (for hit testing mouse events), whereas the others are sim-
 * centric (for getting all entities at a particular location in the world, 
 * with no regard for draw order).
 *
 * Internally, world objects are organized into "cells", each of which has a
 * size corresponding to a configurable cell size. This value can be tweaked
 * to affect performance.
 */
class WorldObjectLocator
{
public:
    WorldObjectLocator();

    // There shouldn't be any reason to copy or move a locator. If it's needed,
    // we can revisit this.
    WorldObjectLocator(const WorldObjectLocator& other) = delete;
    WorldObjectLocator(WorldObjectLocator&& other) = delete;

    /**
     * Adds the given world object to the locator.
     *
     * The given extent will be stored. To update the object's extent in
     * this locator, call clear() and re-add the object.
     *
     * Note: Objects are layered according to the order that they're added
     *       in. E.g. if 2 overlapping objects are added, the second will
     *       be considered to be in front of the first.
     *
     * Note: Assumes the given object is fully within this locator's extent.
     *       Don't pass in object that are outside its bounds.
     *
     * @param objectID  The object to add.
     * @param objectWorldBounds  The object's world-space bounding box.
     */
    void addWorldObject(const WorldObjectID& objectID,
                        const BoundingBox& objectWorldBounds);

    /**
     * Returns the top-most world object under the given screen-space point.
     * If multiple objects are intersected, the one drawn on top will be
     * returned.
     *
     * @param screenPoint  The point on the screen to hit test with.
     * @return The ID of the intersected object. If no layer is hit, the variant
     *         type will == std::monostate.
     */
    WorldObjectID getObjectUnderPoint(const SDL_Point& screenPoint) const;

    /**
     * Clears all of our internal data structures, getting rid of any tracked
     * objects.
     */
    void clear();

    /**
     * Sets the camera that will be used for screen/world conversions.
     */
    void setCamera(const Camera& inCamera);

    /**
     * Sets the part of the world map that this locator covers.
     *
     * Typically, this will match the view extent of the player's camera.
     *
     * All tracked widgets must be fully within these bounds.
     */
    void setExtent(const TileExtent& inTileExtent);

private:
    /** A simple structure for holding tracked objects. */
    struct WorldObject {
        WorldObjectID objectID{};
        BoundingBox worldBounds{};
    };

    /** The width of a cell in world units. */
    static constexpr float CELL_WORLD_WIDTH{
        Config::WORLD_OBJECT_LOCATOR_CELL_WIDTH
        * SharedConfig::TILE_WORLD_WIDTH};
    /** The height of a cell in world units. */
    static constexpr float CELL_WORLD_HEIGHT{
        Config::WORLD_OBJECT_LOCATOR_CELL_HEIGHT
        * SharedConfig::TILE_WORLD_HEIGHT};

    /** The part of the world map that this locator currently covers, in
        world units. */
    BoundingBox locatorBounds;

    /** The part of the world map that this locator currently covers, with
        cells as the unit. */
    CellExtent locatorCellExtent;

    /** The camera to use for screen/world conversions. */
    Camera camera;

    /** A spatial partitioning hash map, mapping cell positions -> vectors
        containing the world objects that currently intersect with that cell. */
    std::unordered_map<CellPosition, std::vector<WorldObject>> objectMap;
};

} // End namespace Client
} // End namespace AM
