#include "WorldObjectLocator.h"
#include "SDLHelpers.h"
#include "Transforms.h"
#include "entt/entity/entity.hpp"
#include <stdexcept>
#include <cmath>

namespace AM
{
namespace Client
{

WorldObjectLocator::WorldObjectLocator()
: locatorBounds{}
, locatorCellExtent{}
, camera{}
, objectMap{}
{
}

void WorldObjectLocator::addWorldObject(const WorldObjectID& objectID,
                                        const BoundingBox& objectWorldBounds)
{
    // If objectID is empty, fail.
    if (std::get_if<std::monostate>(&objectID)) {
        LOG_ERROR("Tried to use an empty world object ID.");
    }

    // Find the cells that the bounding box intersects.
    CellExtent boxCellExtent(objectWorldBounds, CELL_WORLD_WIDTH,
                             CELL_WORLD_HEIGHT);

    // Make sure each length is at least 1, or else the box won't be added to 
    // any cells.
    // (This occurs if the box forms a plane.)
    if (boxCellExtent.xLength < 1) {
        boxCellExtent.xLength = 1;
    }
    if (boxCellExtent.yLength < 1) {
        boxCellExtent.yLength = 1;
    }
    if (boxCellExtent.zLength < 1) {
        boxCellExtent.zLength = 1;
    }

    // Add the object to all the cells that it occupies.
    for (int z{boxCellExtent.z}; z <= boxCellExtent.zMax(); ++z) {
        for (int y{boxCellExtent.y}; y <= boxCellExtent.yMax(); ++y) {
            for (int x{boxCellExtent.x}; x <= boxCellExtent.xMax(); ++x) {
                std::vector<WorldObject>& objectVec{objectMap[{x, y, z}]};
                objectVec.emplace_back(objectID, objectWorldBounds);
            }
        }
    }
}

WorldObjectID
    WorldObjectLocator::getObjectUnderPoint(const SDL_Point& screenPoint) const
{
    // DDA Algorithm Ref: https://lodev.org/cgtutor/raycasting.html
    //                    https://www.youtube.com/watch?v=NbSee-XM7WA

    // Cast a world-space ray from the plane formed by camera.target.z to the 
    // given point on the screen.
    std::optional<Ray> rayOpt{Transforms::screenToWorldRay(
        SDLHelpers::pointToFPoint(screenPoint), camera)};
    if (!(rayOpt.has_value())) {
        return {};
    }
    Ray& ray{rayOpt.value()};

    // Calc the ratio of how long we have to travel along the ray to
    // fully move through 1 cell in each direction.
    // Note: Since our ray is from our iso camera, X/Y will always be equal.
    const float unitStepX{std::abs(1 / ray.direction.x)};
    const float unitStepZ{std::abs(1 / ray.direction.z)};
    const float cellStepX{unitStepX * CELL_WORLD_WIDTH};
    const float cellStepY{cellStepX};
    const float cellStepZ{unitStepZ * CELL_WORLD_HEIGHT};

    // These hold the value (relative to stepX/Y/Z) where the next
    // intersection occurs in each direction.
    // Note: Dividing by CELL_WORLD_ gives us a ratio of total cell size, 
    //       which we can then multiply by cellStep to get "how much of a 
    //       step would we have to make along the ray to reach the next cell".
    CellPosition currentCellPosition(TilePosition(ray.origin),
                                     Config::WORLD_OBJECT_LOCATOR_CELL_WIDTH,
                                     Config::WORLD_OBJECT_LOCATOR_CELL_HEIGHT);
    float nextIntersectionX{
        (ray.origin.x - (currentCellPosition.x * CELL_WORLD_WIDTH))
        / CELL_WORLD_WIDTH * cellStepX};
    float nextIntersectionY{
        (ray.origin.y - (currentCellPosition.y * CELL_WORLD_WIDTH))
        / CELL_WORLD_WIDTH * cellStepY};
    float nextIntersectionZ{
        (ray.origin.z - (currentCellPosition.z * CELL_WORLD_HEIGHT))
        / CELL_WORLD_HEIGHT * cellStepZ};

    // Find the furthest intersection between the ray and the camera's view 
    // bounds so we know where to stop the walk.
    float furthestT{camera.viewBounds.getMaxIntersection(ray)};
    Position furthestViewIntersection{ray.getPointAtT(furthestT)};
    CellPosition endCellPosition{TilePosition(furthestViewIntersection),
                                 Config::WORLD_OBJECT_LOCATOR_CELL_WIDTH,
                                 Config::WORLD_OBJECT_LOCATOR_CELL_HEIGHT};

    // Walk along the ray, checking each cell for a hit world object.
    // (We iterate until we walk past the end position along some axis).
    while ((currentCellPosition.x >= endCellPosition.x)
           && (currentCellPosition.y >= endCellPosition.y)
           && (currentCellPosition.z >= endCellPosition.z)) {
        // If an object in this cell intersects the ray, return it.
        auto pair{objectMap.find(currentCellPosition)};
        if (pair != objectMap.end()) {
            const std::vector<WorldObject>& objectVector{pair->second};

            // Note: Objects are in render order, so we need to reverse iterate.
            for (auto it{objectVector.rbegin()}; it != objectVector.rend();
                 ++it) {
                const WorldObject& object{*it};
                if (object.worldBounds.intersects(ray)) {
                    return object.objectID;
                }
            }
        }

        // Move towards the next closest cell.
        if ((nextIntersectionX < nextIntersectionY)
            && (nextIntersectionX < nextIntersectionZ)) {
            nextIntersectionX += cellStepX;
            currentCellPosition.x -= 1;
        }
        else if ((nextIntersectionY < nextIntersectionX)
            && (nextIntersectionY < nextIntersectionZ)) {
            nextIntersectionY += cellStepY;
            currentCellPosition.y -= 1;
        }
        else {
            nextIntersectionZ += cellStepZ;
            currentCellPosition.z -= 1;
        }
    }

    // No intersected object found. Return null.
    return {};
}

void WorldObjectLocator::clear()
{
    objectMap.clear();
}

void WorldObjectLocator::setCamera(const Camera& inCamera)
{
    camera = inCamera;
}

void WorldObjectLocator::setExtent(const TileExtent& inTileExtent)
{
    locatorBounds = BoundingBox(inTileExtent);
    // Note: We don't want our rays to immediately intersect with these bounds,
    //       so we drop the Z bound below the extent.
    locatorBounds.min.z -= 0.1f;

    locatorCellExtent
        = CellExtent(inTileExtent, Config::WORLD_OBJECT_LOCATOR_CELL_WIDTH,
                     Config::WORLD_OBJECT_LOCATOR_CELL_HEIGHT);
}

} // End namespace Client
} // End namespace AM
