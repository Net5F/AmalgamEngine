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
SDL_FPoint Transforms::worldToScreen(const Vector3& point, float zoomFactor)
{
    // Convert cartesian world point to isometric screen point.
    float screenX{(point.x - point.y)
                  * (TILE_FACE_WIDTH_WORLD_TO_SCREEN / 2.f)};
    float screenY{(point.x + point.y)
                  * (TILE_FACE_HEIGHT_WORLD_TO_SCREEN / 2.f)};

    // The Z coordinate contribution is independent of X/Y and only affects the
    // screen's Y axis. Scale and apply it.
    screenY -= (point.z * TILE_SIDE_HEIGHT_WORLD_TO_SCREEN);

    // Apply the camera zoom.
    screenX *= zoomFactor;
    screenY *= zoomFactor;

    return {screenX, screenY};
}

float Transforms::worldZToScreenY(float zCoord, float zoomFactor)
{
    return zCoord * zoomFactor * TILE_SIDE_HEIGHT_WORLD_TO_SCREEN;
}

Vector3 Transforms::screenToWorldMinimum(const SDL_FPoint& screenPoint,
                                         const Camera& camera)
{
    // Offset the screen point to include the camera position.
    SDL_FPoint absolutePoint{};
    absolutePoint.x = screenPoint.x + camera.screenExtent.x;
    absolutePoint.y = screenPoint.y + camera.screenExtent.y;

    // Remove the camera zoom.
    float x{absolutePoint.x / camera.zoomFactor};
    float y{absolutePoint.y / camera.zoomFactor};

    // Calc the world point.
    float worldX{((2.f * y) + x) * TILE_FACE_WIDTH_SCREEN_TO_WORLD};
    float worldY{((2.f * y) - x) * TILE_FACE_HEIGHT_SCREEN_TO_WORLD / 2.f};

    return {worldX, worldY, 0};
}

Vector3 Transforms::screenToWorldTarget(const SDL_FPoint& screenPoint,
                                        const Camera& camera)
{
    // Find the T where a ray cast from screenPoint intersects the camera 
    // target's Z plane.
    Ray ray{screenToWorldRay(screenPoint, camera)};
    BoundingBox zPlane{{-1'000'000.f, -1'000'000.f, -0.1f},
                       {1'000'000.f, 1'000'000.f, camera.target.z}};
    float intersectT{zPlane.getMinIntersection(ray)};
    AM_ASSERT(intersectT > 0, "Screen ray failed to intersect Z plane.");

    // Return the intersected point.
    return ray.getPointAtT(intersectT);
}

Ray Transforms::screenToWorldRay(const SDL_FPoint& screenPoint,
                                 const Camera& camera)
{
    // Ref: https://gamedev.stackexchange.com/a/206067/124282

    // Find where screenPoint intersects the world at Z == 0.
    Vector3 minimum{screenToWorldMinimum(screenPoint, camera)};

    // Cast a ray up from the minimum point towards the camera.
    // Find the furthest point where this ray intersects the camera's bounds.
    Ray rayToCamera{minimum,
                    {TILE_SIDE_HEIGHT_WORLD_TO_SCREEN,
                     TILE_SIDE_HEIGHT_WORLD_TO_SCREEN,
                     TILE_FACE_HEIGHT_WORLD_TO_SCREEN}};
    rayToCamera.direction.normalize();
    float tMax{camera.viewBounds.getMaxIntersection(rayToCamera)};
    AM_ASSERT(tMax > 0, "Screen ray failed to intersect camera bounds.");

    Vector3 viewBoundsIntersection{rayToCamera.getPointAtT(tMax)};

    // Return a ray that starts at the intersected position and points towards 
    // the minimum.
    return {viewBoundsIntersection,
            -rayToCamera.direction.x,
            -rayToCamera.direction.y,
            -rayToCamera.direction.z};
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
    BoundingBox zPlane{{-1'000'000.f, -1'000'000.f, -0.1f},
                       {1'000'000.f, 1'000'000.f, tileZWorld}};
    float intersectT{zPlane.getMinIntersection(ray)};
    AM_ASSERT(intersectT > 0, "Screen ray failed to intersect Z plane.");

    // Return the intersected tile position.
    Vector3 intersectPoint{ray.getPointAtT(intersectT)};
    return TilePosition{intersectPoint};
}

float Transforms::screenYToWorldZ(float yCoord, float zoomFactor)
{
    return (yCoord / zoomFactor) * TILE_SIDE_HEIGHT_SCREEN_TO_WORLD;
}

BoundingBox Transforms::modelToWorldTile(const BoundingBox& modelBounds,
                                         const TilePosition& tilePosition)
{
    static constexpr float TILE_WORLD_WIDTH{SharedConfig::TILE_WORLD_WIDTH};
    static constexpr float TILE_WORLD_HEIGHT{SharedConfig::TILE_WORLD_HEIGHT};

    // Offset the model-space bounding box to align it with the tile
    Vector3 offset{tilePosition.x * TILE_WORLD_WIDTH,
                   tilePosition.y * TILE_WORLD_WIDTH,
                   tilePosition.z * TILE_WORLD_HEIGHT};

    return modelBounds.translateBy(offset);
}

BoundingBox Transforms::modelToWorldEntity(const BoundingBox& modelBounds,
                                           const Position& position)
{
    // Entities should have their IdleSouth bounding box's bottom center
    // aligned with their Position.
    BoundingBox newBounds{modelBounds};
    newBounds = newBounds.moveBottomCenterTo(position);

    return newBounds;
}

} // End namespace AM
