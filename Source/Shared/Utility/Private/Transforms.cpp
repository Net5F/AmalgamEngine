#include "Transforms.h"
#include "Camera.h"
#include "SharedConfig.h"
#include "Log.h"
#include <cmath>

namespace AM
{
ScreenPoint Transforms::worldToScreen(const Position position, float zoomFactor)
{
    // Calc the scaling factor going from world tiles to screen tiles.
    static const float TILE_WIDTH_SCALE
        = static_cast<float>(SharedConfig::TILE_SCREEN_WIDTH)
          / SharedConfig::TILE_WORLD_WIDTH;
    static const float TILE_HEIGHT_SCALE
        = static_cast<float>(SharedConfig::TILE_SCREEN_HEIGHT)
          / SharedConfig::TILE_WORLD_HEIGHT;

    // Convert cartesian world point to isometric screen point.
    float screenX = (position.x - position.y) * (TILE_WIDTH_SCALE / 2.f);
    float screenY = (position.x + position.y) * (TILE_HEIGHT_SCALE / 2.f);

    // The Z coordinate scaling is independent of X/Y and only affects the
    // screen's Y axis. Scale and apply it.
    screenY -= (position.z * SharedConfig::Z_SCREEN_SCALE);

    // Apply the camera zoom.
    screenX *= zoomFactor;
    screenY *= zoomFactor;

    return {screenX, screenY};
}

float Transforms::worldZToScreenY(float zCoord, float zoomFactor)
{
    return zCoord * zoomFactor * SharedConfig::Z_SCREEN_SCALE;
}

Position Transforms::screenToWorld(const ScreenPoint screenPoint,
                                   float zoomFactor)
{
    // Remove the camera zoom.
    float x = screenPoint.x / zoomFactor;
    float y = screenPoint.y / zoomFactor;

    // Calc the scaling factor going from screen tiles to world tiles.
    static const float TILE_WIDTH_SCALE
        = static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
          / SharedConfig::TILE_SCREEN_WIDTH;
    static const float TILE_HEIGHT_SCALE
        = static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)
          / SharedConfig::TILE_SCREEN_HEIGHT;

    // Calc the world position.
    float worldX = ((2.f * y) + x) * TILE_WIDTH_SCALE;
    float worldY = ((2.f * y) - x) * TILE_HEIGHT_SCALE / 2.f;

    // TODO: Figure out how to handle Z.
    return {worldX, worldY, 0};
}

float Transforms::screenYToWorldZ(float yCoord, float zoomFactor)
{
    // Calc the scaling factor going from screen Y units to world Z units.
    static const float Z_WORLD_SCALE = 1 / SharedConfig::Z_SCREEN_SCALE;

    return yCoord * zoomFactor * Z_WORLD_SCALE;
}

TileIndex Transforms::worldToTile(const Position& position)
{
    // Our tiles are 2D, so Z doesn't matter.
    return {static_cast<int>(position.x / SharedConfig::TILE_WORLD_WIDTH),
            static_cast<int>(position.y / SharedConfig::TILE_WORLD_HEIGHT)};
}

TileIndex Transforms::screenToTile(const ScreenPoint& screenPoint,
                                   const Camera& camera)
{
    // Remove the camera adjustment.
    ScreenPoint absolutePoint;
    absolutePoint.x = screenPoint.x + camera.extent.x;
    absolutePoint.y = screenPoint.y + camera.extent.y;

    // Convert to world space.
    Position worldPos = screenToWorld(absolutePoint, camera.zoomFactor);

    // Convert to tile index.
    return worldToTile(worldPos);
}

} // End namespace AM
