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
    CellExtent boxCellExtent{};
    boxCellExtent.x = static_cast<int>(
        std::floor(objectWorldBounds.minX / CELL_WORLD_WIDTH));
    boxCellExtent.y = static_cast<int>(
        std::floor(objectWorldBounds.minY / CELL_WORLD_WIDTH));
    boxCellExtent.z = static_cast<int>(
        std::floor(objectWorldBounds.minZ / CELL_WORLD_HEIGHT));
    boxCellExtent.xLength = (static_cast<int>(std::ceil(objectWorldBounds.maxX
                                                        / CELL_WORLD_WIDTH))
                             - boxCellExtent.x);
    boxCellExtent.yLength = (static_cast<int>(std::ceil(objectWorldBounds.maxY
                                                        / CELL_WORLD_WIDTH))
                             - boxCellExtent.y);
    boxCellExtent.zLength = (static_cast<int>(std::ceil(objectWorldBounds.maxZ
                                                        / CELL_WORLD_HEIGHT))
                             - boxCellExtent.z);

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
    Ray rayToCamera{Transforms::screenToWorldRay(
        SDLHelpers::pointToFPoint(screenPoint), camera)};
    rayToCamera.normalize();

    // Find where the ray intersects this locator's extent.
    float intersectT{locatorBounds.intersects(rayToCamera)};
    if (intersectT == -1) {
        // If there's no intersection, return empty.
        return {};
    }

    // Calc the ray we'll be tracing, from locator bounds -> the camera Z plane.
    Position rayOrigin{rayToCamera.getPositionAtT(intersectT)};
    Ray ray{{rayOrigin.x, rayOrigin.y, rayOrigin.z},
            -rayToCamera.directionX,
            -rayToCamera.directionY,
            -rayToCamera.directionZ};

    // Calc the ratio of how long we have to travel along the ray to
    // fully move through 1 cell in each direction.
    // Note: Since our ray is from our iso camera, X/Y will always be equal.
    const float unitStepX{std::abs(1 / ray.directionX)};
    const float unitStepZ{std::abs(1 / ray.directionZ)};
    const float cellStepX{unitStepX * CELL_WORLD_WIDTH};
    const float cellStepY{cellStepX};
    const float cellStepZ{unitStepZ * CELL_WORLD_HEIGHT};

    // These hold the value (relative to stepX/Y/Z) where the next
    // intersection occurs in each direction.
    // Note: Dividing by CELL_WORLD_ gives us a ratio of total cell size, 
    //       which we can then multiply by cellStep to get "how much of a 
    //       step would we have to make along the ray to reach the next cell".
    CellPosition currentCellPosition{
        tileToCellPosition(rayOrigin.asTilePosition())};
    float nextIntersectionX{
        (ray.origin.x - (currentCellPosition.x * CELL_WORLD_WIDTH))
        / CELL_WORLD_WIDTH * cellStepX};
    float nextIntersectionY{
        (ray.origin.y - (currentCellPosition.y * CELL_WORLD_WIDTH))
        / CELL_WORLD_WIDTH * cellStepY};
    float nextIntersectionZ{
        (ray.origin.z - (currentCellPosition.z * CELL_WORLD_HEIGHT))
        / CELL_WORLD_HEIGHT * cellStepZ};

    // Walk along the ray, checking each cell for a hit world object.
    CellPosition endCellPosition{
        tileToCellPosition(rayToCamera.origin.asTilePosition())};
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
                if (object.worldBounds.intersects(ray) != -1) {
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
    //       so we drop the Z bound below the extent.
    locatorBounds.minZ
        = (inTileExtent.z * static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT))
          - 0.1f;
    locatorBounds.maxZ = (inTileExtent.z + inTileExtent.zLength)
                         * static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT);

    locatorCellExtent = tileToCellExtent(inTileExtent);
}

CellPosition WorldObjectLocator::tileToCellPosition(
    const TilePosition& tilePosition) const
{
    // Cast constants to a float so we get float division below.
    static constexpr float CELL_WIDTH{Config::WORLD_OBJECT_LOCATOR_CELL_WIDTH};
    static constexpr float CELL_HEIGHT{
        Config::WORLD_OBJECT_LOCATOR_CELL_HEIGHT};

    return {static_cast<int>(std::floor(tilePosition.x / CELL_WIDTH)),
            static_cast<int>(std::floor(tilePosition.y / CELL_WIDTH)),
            static_cast<int>(std::floor(tilePosition.z / CELL_HEIGHT))};
}

CellExtent WorldObjectLocator::tileToCellExtent(const TileExtent& tileExtent)
{
    // Cast constants to a float so we get float division below.
    static constexpr float CELL_WIDTH{Config::WORLD_OBJECT_LOCATOR_CELL_WIDTH};
    static constexpr float CELL_HEIGHT{
        Config::WORLD_OBJECT_LOCATOR_CELL_HEIGHT};

    CellPosition origin{};
    origin.x = static_cast<int>(std::floor(tileExtent.x / CELL_WIDTH));
    origin.y = static_cast<int>(std::floor(tileExtent.y / CELL_WIDTH));
    origin.z = static_cast<int>(std::floor(tileExtent.z / CELL_HEIGHT));

    CellPosition extreme{};
    extreme.x = static_cast<int>(
        std::ceil((tileExtent.x + tileExtent.xLength) / CELL_WIDTH));
    extreme.y = static_cast<int>(
        std::ceil((tileExtent.y + tileExtent.yLength) / CELL_WIDTH));
    extreme.z = static_cast<int>(
        std::ceil((tileExtent.z + tileExtent.zLength) / CELL_HEIGHT));

    return {origin.x,
            origin.y,
            origin.z,
            (extreme.x - origin.x),
            (extreme.y - origin.y),
            (extreme.z - origin.z)};
}

} // End namespace Client
} // End namespace AM
