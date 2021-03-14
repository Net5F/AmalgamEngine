#include "TransformationHelpers.h"
#include "Camera.h"
#include "Sprite.h"
#include "SimDefs.h"
#include "Log.h"
#include <cmath>

namespace AM
{
namespace Client
{
ScreenPoint TransformationHelpers::worldToScreen(const Position position,
                                                 const float zoom)
{
    // Calc the scaling factor going from world tiles to screen tiles.
    static const float TILE_WIDTH_SCALE
        = static_cast<float>(TILE_SCREEN_WIDTH) / TILE_WORLD_WIDTH;
    static const float TILE_HEIGHT_SCALE
        = static_cast<float>(TILE_SCREEN_HEIGHT) / TILE_WORLD_HEIGHT;

    // Convert cartesian world point to isometric screen point.
    float screenX = (position.x - position.y) * (TILE_WIDTH_SCALE / 2.f);
    float screenY
        = (position.x + position.y - position.z) * (TILE_HEIGHT_SCALE / 2.f);

    // Apply the camera zoom.
    screenX *= zoom;
    screenY *= zoom;

    return {screenX, screenY};
}

Position TransformationHelpers::screenToWorld(const ScreenPoint screenPoint,
                                              const float zoom)
{
    // Remove the camera zoom.
    float x = screenPoint.x / zoom;
    float y = screenPoint.y / zoom;

    // Calc the scaling factor going from screen tiles to world tiles.
    static const float TILE_WIDTH_SCALE
        = static_cast<float>(TILE_WORLD_WIDTH) / TILE_SCREEN_WIDTH;
    static const float TILE_HEIGHT_SCALE
        = static_cast<float>(TILE_WORLD_HEIGHT) / TILE_SCREEN_HEIGHT;

    // Calc the world position.
    float worldX = ((2 * y) + x) * (TILE_WIDTH_SCALE);
    float worldY = ((2 * y) - x) * (TILE_HEIGHT_SCALE) / 2;

    // TODO: Figure out how to handle Z.
    return {worldX, worldY, 0};
}

SDL2pp::Rect TransformationHelpers::worldToScreenExtent(
    const Position& position, const Camera& camera, const Sprite& sprite)
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

SDL2pp::Rect TransformationHelpers::tileToScreenExtent(const TileIndex& index,
                                                       const Camera& camera,
                                                       const Sprite& sprite)
{
    // Convert tile index to isometric screen position.
    float screenX = (index.x - index.y) * (TILE_SCREEN_WIDTH / 2.f);
    float screenY = (index.x + index.y) * (TILE_SCREEN_HEIGHT / 2.f);

    // In an iso view, the (0, 0) point of a tile is halfway through the width
    // of the sprite. Thus, we have to shift the tile back to align it.
    screenX -= (TILE_SCREEN_WIDTH / 2.f);

    // Apply the camera zoom to the position.
    screenX *= camera.zoomFactor;
    screenY *= camera.zoomFactor;

    // Get the sprite's vertical offset (iso sprites may have extra
    // vertical space to show depth, we just want the tile.)
    float spriteOffsetY
        = (sprite.height - TILE_SCREEN_HEIGHT - TILE_SCREEN_EDGE_HEIGHT)
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
    return {static_cast<int>(position.x / TILE_WORLD_WIDTH),
            static_cast<int>(position.y / TILE_WORLD_HEIGHT)};
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

} // End namespace Client
} // End namespace AM
