#include "ClientTransforms.h"
#include "Transforms.h"
#include "Camera.h"
#include "Sprite.h"
#include "SharedConfig.h"
#include "Log.h"
#include <cmath>

namespace AM
{
namespace Client
{
SDL_Rect ClientTransforms::worldToScreenExtent(const Position& position,
                                               const Sprite& sprite,
                                               const Camera& camera)
{
    // Get the point in screen space.
    ScreenPoint screenPoint
        = Transforms::worldToScreen(position, camera.zoomFactor);

    // Apply the camera position adjustment.
    int adjustedX
        = static_cast<int>(std::round(screenPoint.x - camera.extent.x));
    int adjustedY
        = static_cast<int>(std::round(screenPoint.y - camera.extent.y));

    // Apply the camera's zoom to the sprite size.
    int zoomedWidth
        = static_cast<int>(std::round(sprite.textureExtent.w * camera.zoomFactor));
    int zoomedHeight
        = static_cast<int>(std::round(sprite.textureExtent.h * camera.zoomFactor));

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

SDL_Rect ClientTransforms::tileToScreenExtent(const TileIndex& index,
                                              const Sprite& sprite,
                                              const Camera& camera)
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
    float spriteOffsetY = (sprite.textureExtent.h - SharedConfig::TILE_SCREEN_HEIGHT
                           - SharedConfig::TILE_SCREEN_EDGE_HEIGHT)
                          * camera.zoomFactor;

    // Apply the camera adjustment.
    int adjustedX = static_cast<int>(std::round(screenX - camera.extent.x));
    int adjustedY = static_cast<int>(
        std::round(screenY - spriteOffsetY - camera.extent.y));

    // Apply the camera's zoom to the sprite size.
    int zoomedWidth
        = static_cast<int>(std::round(sprite.textureExtent.w * camera.zoomFactor));
    int zoomedHeight
        = static_cast<int>(std::round(sprite.textureExtent.h * camera.zoomFactor));

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

} // End namespace Client
} // End namespace AM
