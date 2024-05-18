#include "Transforms.h"
#include "Position.h"
#include "Camera.h"
#include "BoundingBox.h"
#include "Sprite.h"
#include "SharedConfig.h"
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

Position Transforms::screenToWorld(const SDL_FPoint& screenPoint,
                                   const Camera& camera)
{
    // Offset the screen point to include the camera position.
    SDL_FPoint absolutePoint{};
    absolutePoint.x = screenPoint.x + camera.extent.x;
    absolutePoint.y = screenPoint.y + camera.extent.y;

    // Remove the camera zoom.
    float x{absolutePoint.x / camera.zoomFactor};
    float y{absolutePoint.y / camera.zoomFactor};

    // Offset the point to be relative to the closest world Z level that the 
    // camera's target is above. I.e., treat whatever Z level the target is 
    // standing above as the ground plane.
    int zTileOffset{
        static_cast<int>(camera.target.z / SharedConfig::TILE_WORLD_HEIGHT)};
    float zScreenOffset{zTileOffset * SharedConfig::TILE_WORLD_HEIGHT
                        * TILE_SIDE_HEIGHT_WORLD_TO_SCREEN};
    y += zScreenOffset;

    // Calc the world position.
    float worldX{((2.f * y) + x) * TILE_FACE_WIDTH_SCREEN_TO_WORLD};
    float worldY{((2.f * y) - x) * TILE_FACE_HEIGHT_SCREEN_TO_WORLD / 2.f};

    return {worldX, worldY, camera.target.z};
}

Ray Transforms::screenToWorldRay(const SDL_FPoint& screenPoint,
                                 const Camera& camera)
{
    // Ref: https://gamedev.stackexchange.com/a/206067/124282

    // Find where the screen point intersects the world at camera.target.z.
    Position floorPos{screenToWorld(screenPoint, camera)};

    // Return a ray that starts at the calculated position and points towards
    // the camera.
    return {{floorPos.x, floorPos.y, floorPos.z},
            TILE_SIDE_HEIGHT_WORLD_TO_SCREEN,
            TILE_SIDE_HEIGHT_WORLD_TO_SCREEN,
            TILE_FACE_HEIGHT_WORLD_TO_SCREEN};
}

float Transforms::screenYToWorldZ(float yCoord, float zoomFactor)
{
    return (yCoord / zoomFactor) * TILE_SIDE_HEIGHT_SCREEN_TO_WORLD;
}

TilePosition Transforms::screenToTile(const SDL_FPoint& screenPoint,
                                      const Camera& camera)
{
    // Convert to world space.
    Position worldPos{screenToWorld(screenPoint, camera)};

    // Convert to tile position.
    return worldPos.asTilePosition();
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

Position Transforms::boundsToEntityPosition(const BoundingBox& boundingBox,
                                            const Sprite& sprite)
{
    // The box is centered on the entity's position by offsetting it by
    // half of the sprite's stage size. Remove this stage offset and the
    // model offset to find the X/Y position.
    // Note: This assumes that the sprite's stage is 1 tile large. When we
    //       add support for other sizes, this will need to be updated.
    Position position{};
    position.x = boundingBox.minX - sprite.modelBounds.minX
                 + (SharedConfig::TILE_WORLD_WIDTH / 2);
    position.y = boundingBox.minY - sprite.modelBounds.minY
                 + (SharedConfig::TILE_WORLD_WIDTH / 2);

    // The bottom of the stage is flush with the entity's position, so we
    // only need to remove the model offset.
    position.z = boundingBox.minZ - sprite.modelBounds.minZ;

    return position;
}

} // End namespace AM
