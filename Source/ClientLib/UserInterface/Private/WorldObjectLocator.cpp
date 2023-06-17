#include "WorldObjectLocator.h"
#include "SDLHelpers.h"
#include "entt/entity/entity.hpp"
#include <stdexcept>

namespace AM
{
namespace Client
{

WorldObjectLocator::WorldObjectLocator()
: cellWorldWidth{DEFAULT_CELL_WIDTH}
, objectMap{}
{
}

void WorldObjectLocator::addWorldObject(const WorldObjectIDVariant& objectID,
                                        const BoundingBox& objectWorldBounds)
{
    // If objectID is empty, fail.
    if (std::get_if<std::monostate>(&objectID)) {
        LOG_ERROR("Tried to use an empty world object ID.");
    }

    // Find the cells that the bounding box intersects.
    CellExtent boxCellExtent{};
    boxCellExtent.x
        = static_cast<int>(std::floor(objectWorldBounds.minX / cellWorldWidth));
    boxCellExtent.y
        = static_cast<int>(std::floor(objectWorldBounds.minY / cellWorldWidth));
    boxCellExtent.xLength
        = (static_cast<int>(std::ceil(objectWorldBounds.maxX / cellWorldWidth))
           - boxCellExtent.x);
    boxCellExtent.yLength
        = (static_cast<int>(std::ceil(objectWorldBounds.maxY / cellWorldWidth))
           - boxCellExtent.y);

    // Add the object to all the cells that it occupies.
    for (int x = boxCellExtent.x; x <= boxCellExtent.xMax(); ++x) {
        for (int y = boxCellExtent.y; y <= boxCellExtent.yMax(); ++y) {
            std::vector<WorldObject>& objectVec{objectMap[{x, y}]};
            objectVec.emplace_back(objectID, objectWorldBounds);
        }
    }
}

WorldObjectIDVariant
    WorldObjectLocator::getObjectUnderPoint(const SDL_Point& screenPoint) const
{
    // Cast a world-space ray from the given point on the screen.

    // Iterate through the cells that the ray is intersecting, from near -> 
    // far, checking the sorted objects in each cell for intersections.

    //// Calc the cell that contains the point.
    //CellPosition cellPosition{static_cast<int>(screenPoint.x / cellWorldWidth),
    //                          static_cast<int>(screenPoint.y / cellWorldWidth)};

    //// Iterate the cell's objects to find the first tile layer.
    //try {
    //    const std::vector<WorldObject>& objectVec{objectMap.at(cellPosition)};
    //    for (auto objectIt = objectVec.rbegin(); objectIt != objectVec.rend();
    //         objectIt++) {
    //        // If this object contains the point, return it.
    //        if (SDLHelpers::pointInRect(screenPoint,
    //                                    objectIt->screenExtent)) {
    //            return objectIt->objectID;
    //        }
    //    }
    //} catch (const std::out_of_range&) {
    //    // Let it fall through to the return below.
    //}

    // No intersected object found. Return an empty type.
    return {};
}

TileLayerID
    WorldObjectLocator::getTileLayerUnderPoint(const SDL_Point& screenPoint) const
{
    // Calc the cell that contains the point.
    CellPosition cellPosition{static_cast<int>(screenPoint.x / cellWorldWidth),
                              static_cast<int>(screenPoint.y / cellWorldWidth)};

    //// Iterate the cell's objects to find the first tile layer.
    //try {
    //    const std::vector<WorldObject>& objectVec{objectMap.at(cellPosition)};
    //    for (const WorldObject& worldObject : objectVec) {
    //        // If this object is a tile layer and it contains the point,
    //        // return it.
    //        const TileLayerID* layerID{
    //            std::get_if<TileLayerID>(&(worldObject.objectID))};
    //        if ((layerID != nullptr)
    //            && SDLHelpers::pointInRect(screenPoint,
    //                                       worldObject.screenExtent)) {
    //            return *layerID;
    //        }
    //    }
    //} catch (const std::out_of_range&) {
    //    // Let it fall through to the return below.
    //}

    // No tile layer found at the given point. Return an empty type.
    return TileLayerID{.type = TileLayer::Type::None};
}

entt::entity
    WorldObjectLocator::getEntityUnderPoint(const SDL_Point& screenPoint) const
{
    // Calc the cell that contains the point.
    CellPosition cellPosition{static_cast<int>(screenPoint.x / cellWorldWidth),
                              static_cast<int>(screenPoint.y / cellWorldWidth)};

    //// Iterate the cell's objects to find the first entity.
    //try {
    //    const std::vector<WorldObject>& objectVec{objectMap.at(cellPosition)};
    //    for (const WorldObject& worldObject : objectVec) {
    //        // If this object is an entity and it contains the point,
    //        // return it.
    //        const entt::entity* entityID{
    //            std::get_if<entt::entity>(&(worldObject.objectID))};
    //        if ((entityID != nullptr)
    //            && SDLHelpers::pointInRect(screenPoint,
    //                                       worldObject.screenExtent)) {
    //            return *entityID;
    //        }
    //    }
    //} catch (const std::out_of_range&) {
    //    // Let it fall through to the return below.
    //}

    // No entity found at the given point. Return an empty type.
    return entt::null;
}

void WorldObjectLocator::clear()
{
    objectMap.clear();
}

void WorldObjectLocator::setCellWidth(float inCellWidth)
{
    cellWorldWidth = inCellWidth;
    clear();
}

} // End namespace Client
} // End namespace AM
