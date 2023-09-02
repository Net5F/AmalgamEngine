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
: cellWorldWidth{DEFAULT_CELL_WIDTH}
, locatorBounds{}
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

WorldObjectID
    WorldObjectLocator::getObjectUnderPoint(const SDL_Point& screenPoint) const
{
    // DDA Algorithm Ref: https://lodev.org/cgtutor/raycasting.html
    //                    https://www.youtube.com/watch?v=NbSee-XM7WA

    // Cast a world-space ray from the ground to the given point on the screen.
    Ray rayToCamera{Transforms::screenToWorldRay(SDLHelpers::pointToFPoint(screenPoint),
                                         camera)};
    rayToCamera.normalize();

    // Find where the ray intersects this locator's extent.
    float intersectT{locatorBounds.intersects(rayToCamera)};
    if (intersectT == -1) {
        // If there's no intersection, return empty.
        return {};
    }

    // Calc the ray we'll be tracing, from locator bounds -> the ground.
    Position rayOrigin{rayToCamera.getPositionAtT(intersectT)};
    Ray ray{rayOrigin.x, rayOrigin.y, rayOrigin.z,
            -rayToCamera.directionX, -rayToCamera.directionY,
            -rayToCamera.directionZ};

    // Calc the ratio of how long we have to travel along the ray to 
    // fully move through 1 cell in each direction.
    // Note: Since our ray is from our iso camera, these will always be equal.
    const float stepX{std::abs(1 / ray.directionX)};
    const float stepY{std::abs(1 / ray.directionY)};

    // These hold the value (relative to stepX/stepY) where the next intersection
    // occurs in each direction.
    float nextIntersectionX{
        (std::fmod(ray.origin.x, cellWorldWidth) / cellWorldWidth) * stepX};
    float nextIntersectionY{
        (std::fmod(ray.origin.y, cellWorldWidth) / cellWorldWidth) * stepY};

    // Walk along the ray, checking each cell for a hit world object.
    CellPosition currentCellPosition{
        tileToCellPosition(rayOrigin.asTilePosition())};
    CellPosition endCellPosition{tileToCellPosition(Position{
        rayToCamera.origin.x, rayToCamera.origin.y, rayToCamera.origin.z}
                                                        .asTilePosition())};
    while ((currentCellPosition.x >= endCellPosition.x)
           && (currentCellPosition.y >= endCellPosition.y)) {
        // If an object in this cell intersects the ray, return it.
        auto pair = objectMap.find(currentCellPosition);
        if (pair != objectMap.end()) {
            const std::vector<WorldObject>& objectVector{pair->second};

            // Note: Objects are in render order, so we need to reverse iterate.
            for (auto it = objectVector.rbegin(); it != objectVector.rend();
                 ++it) {
                const WorldObject& object{*it};
                if (object.worldBounds.intersects(ray) != -1) {
                    return object.objectID;
                }
            }
        }

        // Move towards the next closest cell.
        if (nextIntersectionX < nextIntersectionY) {
            nextIntersectionX += stepX;
            currentCellPosition.x -= 1;
        }
        else {
            nextIntersectionY += stepY;
            currentCellPosition.y -= 1;
        }
    }

    // No intersected object found. Return an empty type.
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
    locatorBounds.minX
        = inTileExtent.x * static_cast<float>(SharedConfig::TILE_WORLD_WIDTH);
    locatorBounds.maxX = (inTileExtent.x + inTileExtent.xLength)
                    * static_cast<float>(SharedConfig::TILE_WORLD_WIDTH);
    locatorBounds.minY
        = inTileExtent.y * static_cast<float>(SharedConfig::TILE_WORLD_WIDTH);
    locatorBounds.maxY = (inTileExtent.y + inTileExtent.yLength)
                    * static_cast<float>(SharedConfig::TILE_WORLD_WIDTH);
    // Note: We don't want our rays to immediately intersect with these bounds, 
    //       so we drop the Z bound below the ground.
    locatorBounds.minZ = -0.1f;
    // Note: Our world doesn't have a max Z, so we just make it really large.
    locatorBounds.maxZ = 1'000'000;

    locatorCellExtent = tileToCellExtent(inTileExtent);
}

void WorldObjectLocator::setCellWidth(float inCellWidth)
{
    cellWorldWidth = inCellWidth;
    clear();
}

CellPosition WorldObjectLocator::tileToCellPosition(const TilePosition& tilePosition) const
{
    // Cast CELL_WIDTH to a float so we get float division below.
    float cellWidth{SharedConfig::CELL_WIDTH};

    return {static_cast<int>(std::floor(tilePosition.x / cellWidth)),
            static_cast<int>(std::floor(tilePosition.y / cellWidth))};
}

CellExtent WorldObjectLocator::tileToCellExtent(const TileExtent& tileExtent)
{
    // Cast CELL_WIDTH to a float so we get float division below.
    float cellWidth{SharedConfig::CELL_WIDTH};

    CellPosition topLeft{};
    topLeft.x = static_cast<int>(std::floor(tileExtent.x / cellWidth));
    topLeft.y = static_cast<int>(std::floor(tileExtent.y / cellWidth));

    CellPosition bottomRight{};
    bottomRight.x = static_cast<int>(
        std::ceil((tileExtent.x + tileExtent.xLength) / cellWidth));
    bottomRight.y = static_cast<int>(
        std::ceil((tileExtent.y + tileExtent.yLength) / cellWidth));

    return {topLeft.x, topLeft.y, (bottomRight.x - topLeft.x),
            (bottomRight.y - topLeft.y)};
}

} // End namespace Client
} // End namespace AM
