#include "ClientTransforms.h"
#include "Transforms.h"
#include "Camera.h"
#include "Sprite.h"
#include "SpriteRenderData.h"
#include "SharedConfig.h"
#include "Log.h"

namespace AM
{
namespace Client
{
SDL_FRect
    ClientTransforms::entityToScreenExtent(const Position& position,
                                           const SpriteRenderData& renderData,
                                           const Camera& camera)
{
    // Transform the position to a point in screen space.
    // Note: This applies the camera's zoom to the position, so we don't need
    //       to do it again.
    SDL_FPoint screenPoint{
        Transforms::worldToScreen(position, camera.zoomFactor)};

    // Offset the sprite horizontally to line up with our tile positioning.
    // Note: We assume the sprite's x = 0 point is in its horizontal center.
    screenPoint.x -= ((renderData.textureExtent.w / 2.f) * camera.zoomFactor);

    // An iso sprite may have extra vertical space to show depth, we subtract
    // that space to align it.
    screenPoint.y -= (renderData.yOffset * camera.zoomFactor);

    // screenPoint currently would give us a rect that starts at the given
    // position instead of being centered on it. Pull the point back by a half
    // tile to center the rect.
    // Note: This assumes that the sprite is 1 tile large. When we add support
    //       for other sizes, this will need to be updated.
    screenPoint.y
        -= ((SharedConfig::TILE_FACE_SCREEN_HEIGHT / 2.f) * camera.zoomFactor);

    // Apply the camera position adjustment.
    float adjustedX{screenPoint.x - camera.extent.x};
    float adjustedY{screenPoint.y - camera.extent.y};

    // Apply the camera's zoom to the sprite size.
    float zoomedWidth{renderData.textureExtent.w * camera.zoomFactor};
    float zoomedHeight{renderData.textureExtent.h * camera.zoomFactor};

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

SDL_FRect
    ClientTransforms::tileToScreenExtent(const TilePosition& position,
                                         const SpriteRenderData& renderData,
                                         const Camera& camera)
{
    // Convert tile position to isometric screen point.
    float screenX{(position.x - position.y)
                  * (SharedConfig::TILE_FACE_SCREEN_WIDTH / 2.f)};
    float screenY{(position.x + position.y)
                  * (SharedConfig::TILE_FACE_SCREEN_HEIGHT / 2.f)};

    // The Z coordinate contribution is independent of X/Y and only affects the
    // screen's Y axis. Scale and apply it.
    screenY -= (position.z * SharedConfig::TILE_WORLD_HEIGHT
                * Transforms::TILE_SIDE_HEIGHT_WORLD_TO_SCREEN);

    // In an iso view, the (0, 0) point of a tile is halfway through the width
    // of the sprite. Thus, we have to shift the tile back to align it.
    screenX -= (SharedConfig::TILE_FACE_SCREEN_WIDTH / 2.f);

    // An iso sprite may have extra vertical space to show depth, we subtract
    // that space to align it.
    screenY -= renderData.yOffset;

    // Apply the camera zoom.
    screenX *= camera.zoomFactor;
    screenY *= camera.zoomFactor;

    // Apply the camera adjustment.
    float adjustedX{screenX - camera.extent.x};
    float adjustedY{screenY - camera.extent.y};

    // Apply the camera's zoom to the tile size.
    float zoomedWidth{renderData.textureExtent.w * camera.zoomFactor};
    float zoomedHeight{renderData.textureExtent.h * camera.zoomFactor};

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

} // End namespace Client
} // End namespace AM
