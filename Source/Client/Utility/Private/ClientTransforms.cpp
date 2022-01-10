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
SDL_Rect ClientTransforms::entityToScreenExtent(const Position& position,
                                                const Sprite& sprite,
                                                const Camera& camera)
{
    // Transform the position to a point in screen space.
    // Note: This applies the camera's zoom to the position, so we don't need
    //       to do it again.
    ScreenPoint screenPoint{
        Transforms::worldToScreen(position, camera.zoomFactor)};

    // Offset the sprite horizontally to line up with our tile positioning.
    // Note: We assume the sprite's x = 0 point is in its horizontal center.
    screenPoint.x -= ((sprite.textureExtent.w / 2.f) * camera.zoomFactor);

    // An iso sprite may have extra vertical space to show depth, we subtract
    // that space to align it.
    screenPoint.y -= (sprite.yOffset * camera.zoomFactor);

    // screenPoint currently would give us a rect that starts at the given
    // position instead of being centered on it. Pull the point back by a half
    // tile to center the rect.
    // Note: This assumes that the sprite is 1 tile large. When we add support
    //       for other sizes, this will need to be updated.
    screenPoint.y -= ((SharedConfig::TILE_SCREEN_HEIGHT / 2.f) * camera.zoomFactor);

    // Apply the camera position adjustment.
    int adjustedX{
        static_cast<int>(std::round(screenPoint.x - camera.extent.x))};
    int adjustedY{
        static_cast<int>(std::round(screenPoint.y - camera.extent.y))};

    // Apply the camera's zoom to the sprite size.
    int zoomedWidth{static_cast<int>(
        std::round(sprite.textureExtent.w * camera.zoomFactor))};
    int zoomedHeight{static_cast<int>(
        std::round(sprite.textureExtent.h * camera.zoomFactor))};

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

SDL_Rect ClientTransforms::tileToScreenExtent(const TilePosition& position,
                                              const Sprite& sprite,
                                              const Camera& camera)
{
    // Convert tile position to isometric screen position.
    float screenX{
        (position.x - position.y) * (SharedConfig::TILE_SCREEN_WIDTH / 2.f)};
    float screenY{
        (position.x + position.y) * (SharedConfig::TILE_SCREEN_HEIGHT / 2.f)};

    // In an iso view, the (0, 0) point of a tile is halfway through the width
    // of the sprite. Thus, we have to shift the tile back to align it.
    screenX -= (SharedConfig::TILE_SCREEN_WIDTH / 2.f);

    // An iso sprite may have extra vertical space to show depth, we subtract
    // that space to align it.
    screenY -= sprite.yOffset;

    // Apply the camera zoom.
    screenX *= camera.zoomFactor;
    screenY *= camera.zoomFactor;

    // Apply the camera adjustment.
    int adjustedX{static_cast<int>(std::round(screenX - camera.extent.x))};
    int adjustedY{static_cast<int>(std::round(screenY - camera.extent.y))};

    // Apply the camera's zoom to the tile size.
    int zoomedWidth{static_cast<int>(
        std::round(sprite.textureExtent.w * camera.zoomFactor))};
    int zoomedHeight{static_cast<int>(
        std::round(sprite.textureExtent.h * camera.zoomFactor))};

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

} // End namespace Client
} // End namespace AM
