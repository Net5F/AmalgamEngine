#pragma once

#include "Camera.h"
#include "WorldObjectIDVariant.h"
#include "CellExtent.h"
#include "CellPosition.h"
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
 * In constract to the similar EntityLocator, this class is UI-centric (for 
 * hit testing mouse events), whereas EntityLocator is sim-centric (for getting 
 * all entities at a particular location in the world, with no regard for 
 * draw order).
 *
 * Internally, world objects are organized into "cells", each of which has a 
 * size corresponding to a configurable cell width. This value can be tweaked 
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
    void addWorldObject(const WorldObjectIDVariant& objectID,
                        const BoundingBox& objectWorldBounds);

    WorldObjectIDVariant getObjectUnderPoint(const SDL_Point& screenPoint) const;

    /**
     * Returns the tile layer underneath the given actual-space point.
     * If multiple tile layers are intersected, the one drawn on top will be 
     * returned.
     *
     * @param screenPoint  The point in isometric screen space to hit test with.
     *                     Note: This should include the camera offset.
     * @return The ID of the intersected layer. If no layer is hit, type will 
     *         == TileLayer::Type::None.
     */
    TileLayerID getTileLayerUnderPoint(const SDL_Point& screenPoint) const;

    /**
     * Returns the entity underneath the given actual-space point.
     * If multiple entities are intersected, the one drawn on top will be 
     * returned.
     *
     * @param screenPoint  The point in isometric screen space to hit test with.
     *                     Note: This should include the camera offset.
     * @return The ID of the intersected entity. If no entity is hit, entt::null
     *         will be returned.
     */
    entt::entity getEntityUnderPoint(const SDL_Point& screenPoint) const;

    /**
     * Clears all of our internal data structures, getting rid of any tracked
     * objects.
     */
    void clear();

    /**
     * Sets the width of the cells in the spatial partitioning hash map and 
     * clears the locator's state (since it's now invalid).
     * Note: This isn't typically necessary, the default value should be fine
     *       in most cases.
     */
    void setCellWidth(float inCellWidth);

private:
    /** A simple structure for holding tracked objects. */
    struct WorldObject
    {
        WorldObjectIDVariant objectID{};
        BoundingBox worldBounds{};
    };

    /** The default width of the cells in the spatial partitioning hash map,
        in world units. */
    static constexpr float DEFAULT_CELL_WIDTH{128};

    /**
     * Converts the given isometric screen-space extent to a cell extent.
     */
    SDL_Rect screenToCellExtent(const SDL_Rect& screenExtent);

    /** The width of a grid cell in world units. */
    float cellWorldWidth;

    /** A spatial partitioning hash map, mapping cell positions -> vectors 
        containing the world objects that currently intersect with that cell. */
    std::unordered_map<CellPosition, std::vector<WorldObject>> objectMap;
};

} // End namespace Client
} // End namespace AM
