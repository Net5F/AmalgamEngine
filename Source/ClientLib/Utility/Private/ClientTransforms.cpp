#include "ClientTransforms.h"
#include "Transforms.h"
#include "Camera.h"
#include "Sprite.h"
#include "SpriteRenderData.h"
#include "Position.h"
#include "TilePosition.h"
#include "TileOffset.h"
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

    // Offset the sprite to line up with where the "stage" starts within the 
    // image.
    screenPoint.x -= (renderData.stageOrigin.x * camera.zoomFactor);
    screenPoint.y -= (renderData.stageOrigin.y * camera.zoomFactor);

    // screenPoint currently would give us a rect that starts at the given
    // position instead of being centered on it. Pull the point back by a half
    // tile to center the rect.
    // Note: This assumes that the sprite is 1 tile large. When we add support
    //       for other sizes, this will need to be updated.
    screenPoint.y
        -= ((SharedConfig::TILE_FACE_SCREEN_HEIGHT / 2.f) * camera.zoomFactor);

    // Apply the camera position adjustment.
    float adjustedX{screenPoint.x - camera.screenExtent.x};
    float adjustedY{screenPoint.y - camera.screenExtent.y};

    // Apply the camera's zoom to the sprite size.
    float zoomedWidth{renderData.textureExtent.w * camera.zoomFactor};
    float zoomedHeight{renderData.textureExtent.h * camera.zoomFactor};

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

SDL_FRect ClientTransforms::tileToScreenExtent(
    const TilePosition& tilePosition, const TileOffset& tileOffset,
    const SpriteRenderData& renderData, const Camera& camera)
{
    // Transform the position to a point in screen space.
    // Note: This applies the camera's zoom to the position, so we don't need
    //       to do it again.
    Vector3 tileOrigin{tilePosition.getOriginPoint()};
    Vector3 worldPoint{tileOrigin.x + static_cast<float>(tileOffset.x),
                       tileOrigin.y + static_cast<float>(tileOffset.y),
                       tileOrigin.z + static_cast<float>(tileOffset.z)};
    SDL_FPoint screenPoint{
        Transforms::worldToScreen(worldPoint, camera.zoomFactor)};

    // Offset the sprite to line up with where the "stage" starts within the 
    // image.
    screenPoint.x -= (renderData.stageOrigin.x * camera.zoomFactor);
    screenPoint.y -= (renderData.stageOrigin.y * camera.zoomFactor);

    // Apply the camera adjustment.
    float adjustedX{screenPoint.x - camera.screenExtent.x};
    float adjustedY{screenPoint.y - camera.screenExtent.y};

    // Apply the camera's zoom to the tile size.
    float zoomedWidth{renderData.textureExtent.w * camera.zoomFactor};
    float zoomedHeight{renderData.textureExtent.h * camera.zoomFactor};

    return {adjustedX, adjustedY, zoomedWidth, zoomedHeight};
}

} // End namespace Client
} // End namespace AM
