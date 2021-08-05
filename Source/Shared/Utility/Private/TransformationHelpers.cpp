#include "TransformationHelpers.h"
#include "Camera.h"
#include "Sprite.h"
#include "SharedConfig.h"
#include "Log.h"
#include <cmath>

namespace AM
{
ScreenPoint TransformationHelpers::worldToScreen(const Position position,
                                                 float zoomFactor)
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

float TransformationHelpers::worldZToScreenY(float zCoord, float zoomFactor)
{
    return zCoord * zoomFactor * SharedConfig::Z_SCREEN_SCALE;
}

Position TransformationHelpers::screenToWorld(const ScreenPoint screenPoint,
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

float TransformationHelpers::screenYToWorldZ(float yCoord, float zoomFactor)
{
    // Calc the scaling factor going from screen Y units to world Z units.
    static const float Z_WORLD_SCALE = 1 / SharedConfig::Z_SCREEN_SCALE;

    return yCoord * zoomFactor * Z_WORLD_SCALE;
}

SDL_Rect TransformationHelpers::worldToScreenExtent(const Position& position,
                                                    const Camera& camera,
                                                    const Sprite& sprite)
{
    // Get the point in screen space.
    ScreenPoint screenPoint = worldToScreen(position, camera.zoomFactor);

    // Apply the camera position adjustment.
    int adjustedX
        = static_cast<int>(std::round(screenPoint.x - camera.extent.x));
    int adjustedY
        = static_cast<int>(std::round(screenPoint.y - camera.extent.y));

    // Apply the camera's zoom to the sprite size.
    int zoomedWidth
        = static_cast<int>(std::round(sprite.width * camera.zoomFactor));
    int zoomedHeight
        = static_cast<int>(std::round(sprite.height * camera.zoomFactor));

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

SDL_Rect TransformationHelpers::tileToScreenExtent(const TileIndex& index,
                                                   const Camera& camera,
                                                   const Sprite& sprite)
{
    // Convert tile index to isometric screen position.
    float screenX
        = (index.x - index.y) * (SharedConfig::TILE_SCREEN_WIDTH / 2.f);
    float screenY
        = (index.x + index.y) * (SharedConfig::TILE_SCREEN_HEIGHT / 2.f);

    // In an iso view, the (0, 0) point of a tile is halfway through the width
    // of the sprite. Thus, we have to shift the tile back to align it.
    screenX -= (SharedConfig::TILE_SCREEN_WIDTH / 2.f);

    // Apply the camera zoom to the position.
    screenX *= camera.zoomFactor;
    screenY *= camera.zoomFactor;

    // Get the sprite's vertical offset (iso sprites may have extra
    // vertical space to show depth, we just want the tile.)
    float spriteOffsetY = (sprite.height - SharedConfig::TILE_SCREEN_HEIGHT
                           - SharedConfig::TILE_SCREEN_EDGE_HEIGHT)
                          * camera.zoomFactor;

    // Apply the camera adjustment.
    int adjustedX = static_cast<int>(std::round(screenX - camera.extent.x));
    int adjustedY = static_cast<int>(
        std::round(screenY - spriteOffsetY - camera.extent.y));

    // Apply the camera's zoom to the sprite size.
    int zoomedWidth
        = static_cast<int>(std::round(sprite.width * camera.zoomFactor));
    int zoomedHeight
        = static_cast<int>(std::round(sprite.height * camera.zoomFactor));

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

TileIndex TransformationHelpers::worldToTile(const Position& position)
{
    // Our tiles are 2D, so Z doesn't matter.
    return {static_cast<int>(position.x / SharedConfig::TILE_WORLD_WIDTH),
            static_cast<int>(position.y / SharedConfig::TILE_WORLD_HEIGHT)};
}

TileIndex TransformationHelpers::screenToTile(const ScreenPoint& screenPoint,
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
