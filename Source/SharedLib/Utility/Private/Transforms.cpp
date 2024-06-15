#include "Transforms.h"
#include "Position.h"
#include "Camera.h"
#include "BoundingBox.h"
#include "Sprite.h"
#include "SharedConfig.h"
#include "AMAssert.h"
#include "Log.h"
#include <cmath>

namespace AM
{
SDL_FPoint Transforms::worldToScreen(const Position& position, float zoomFactor)
{
    // Convert cartesian world point to isometric screen point.
    float screenX{(position.x - position.y)
                  * (TILE_FACE_WIDTH_WORLD_TO_SCREEN / 2.f)};
    float screenY{(position.x + position.y)
                  * (TILE_FACE_HEIGHT_WORLD_TO_SCREEN / 2.f)};

    // The Z coordinate contribution is independent of X/Y and only affects the
    // screen's Y axis. Scale and apply it.
    screenY -= (position.z * TILE_SIDE_HEIGHT_WORLD_TO_SCREEN);

    // Apply the camera zoom.
    screenX *= zoomFactor;
    screenY *= zoomFactor;

    return {screenX, screenY};
}

float Transforms::worldZToScreenY(float zCoord, float zoomFactor)
{
    return zCoord * zoomFactor * TILE_SIDE_HEIGHT_WORLD_TO_SCREEN;
}

Position Transforms::screenToWorldMinimum(const SDL_FPoint& screenPoint,
                                          const Camera& camera)
{
    // Offset the screen point to include the camera position.
    SDL_FPoint absolutePoint{};
    absolutePoint.x = screenPoint.x + camera.screenExtent.x;
    absolutePoint.y = screenPoint.y + camera.screenExtent.y;

    // Remove the camera zoom.
    float x{absolutePoint.x / camera.zoomFactor};
    float y{absolutePoint.y / camera.zoomFactor};

    // Calc the world position.
    float worldX{((2.f * y) + x) * TILE_FACE_WIDTH_SCREEN_TO_WORLD};
    float worldY{((2.f * y) - x) * TILE_FACE_HEIGHT_SCREEN_TO_WORLD / 2.f};

    return {worldX, worldY, 0};
}

Position Transforms::screenToWorldTarget(const SDL_FPoint& screenPoint,
                                         const Camera& camera)
{
    // Find the T where a ray cast from screenPoint intersects the camera 
    // target's Z plane.
    Ray ray{screenToWorldRay(screenPoint, camera)};
    BoundingBox zPlane{-1'000'000.f, 1'000'000.f, -1'000'000.f,
                       1'000'000.f,  -0.1f,      camera.target.z};
    float intersectT{zPlane.getMinIntersection(ray)};
    AM_ASSERT(intersectT > 0, "Screen ray failed to intersect Z plane.");

    // Return the intersected position.
    return ray.getPositionAtT(intersectT);
}

Ray Transforms::screenToWorldRay(const SDL_FPoint& screenPoint,
                                 const Camera& camera)
{
    // Ref: https://gamedev.stackexchange.com/a/206067/124282

    // Find where screenPoint intersects the world at Z == 0.
    Position minimum{screenToWorldMinimum(screenPoint, camera)};

    // Cast a ray up from the minimum position towards the camera.
    // Find the furthest position where this ray intersects the 
    // camera's view bounds.
    Ray rayToCamera{minimum, TILE_SIDE_HEIGHT_WORLD_TO_SCREEN,
                    TILE_SIDE_HEIGHT_WORLD_TO_SCREEN,
                    TILE_FACE_HEIGHT_WORLD_TO_SCREEN};
    rayToCamera.normalize();
    float tMax{camera.viewBounds.getMaxIntersection(rayToCamera)};
    AM_ASSERT(tMax > 0, "Screen ray failed to intersect camera bounds.");

    Position viewBoundsIntersection{rayToCamera.getPositionAtT(tMax)};

    // Return a ray that starts at the intersected position and points towards 
    // the minimum.
    return {viewBoundsIntersection,
            -rayToCamera.directionX,
            -rayToCamera.directionY,
            -rayToCamera.directionZ};
}

TilePosition Transforms::screenToWorldTile(const SDL_FPoint& screenPoint,
                                           const Camera& camera)
{
    // Find the Z position of the highest tile that the camera's target is 
    // standing above.
    int tileZCoord{
        static_cast<int>(camera.target.z / SharedConfig::TILE_WORLD_HEIGHT)};
    float tileZWorld{static_cast<float>(tileZCoord)
                     * SharedConfig::TILE_WORLD_HEIGHT};

    // Find the T where a ray cast from screenPoint intersects the tile's 
    // Z plane.
    Ray ray{screenToWorldRay(screenPoint, camera)};
    BoundingBox zPlane{-1'000'000.f, 1'000'000.f, -1'000'000.f,
                       1'000'000.f,  -0.1f,      tileZWorld};
    float intersectT{zPlane.getMinIntersection(ray)};
    AM_ASSERT(intersectT > 0, "Screen ray failed to intersect Z plane.");

    // Return the intersected tile position.
    Position intersectPos{ray.getPositionAtT(intersectT)};
    return intersectPos.asTilePosition();
}

float Transforms::screenYToWorldZ(float yCoord, float zoomFactor)
{
    return (yCoord / zoomFactor) * TILE_SIDE_HEIGHT_SCREEN_TO_WORLD;
}

BoundingBox Transforms::modelToWorld(const BoundingBox& modelBounds,
                                     const Position& position)
{
    // Place the model-space bounding box at the given position.
    BoundingBox movedBox{};
    movedBox.minX = position.x + modelBounds.minX;
    movedBox.maxX = position.x + modelBounds.maxX;
    movedBox.minY = position.y + modelBounds.minY;
    movedBox.maxY = position.y + modelBounds.maxY;
    movedBox.minZ = position.z + modelBounds.minZ;
    movedBox.maxZ = position.z + modelBounds.maxZ;

    return movedBox;
}

BoundingBox Transforms::modelToWorldCentered(const BoundingBox& modelBounds,
                                             const Position& position)
{
    // Place the model-space bounding box at the given position, offset back
    // by half of the sprite's stage size to center it in the X/Y.
    // Note: This assumes that the sprite's stage is 1 tile large. When we add
    //       support for other sizes, this will need to be updated.
    BoundingBox offsetBounds{};
    offsetBounds.minX
        = position.x + modelBounds.minX - (SharedConfig::TILE_WORLD_WIDTH / 2);
    offsetBounds.minY
        = position.y + modelBounds.minY - (SharedConfig::TILE_WORLD_WIDTH / 2);
    offsetBounds.maxX = offsetBounds.minX + modelBounds.getXLength();
    offsetBounds.maxY = offsetBounds.minY + modelBounds.getYLength();

    // Place the bottom of the stage flush with the given position.
    // (e.g. if this is an entity's position, the bottom of the stage is at
    // their feet).
    offsetBounds.minZ = position.z + modelBounds.minZ;
    offsetBounds.maxZ = position.z + modelBounds.maxZ;

    return offsetBounds;
}

} // End namespace AM
