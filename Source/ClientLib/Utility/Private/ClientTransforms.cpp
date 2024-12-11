#include "ClientTransforms.h"
#include "Transforms.h"
#include "Camera.h"
#include "Sprite.h"
#include "SpriteRenderData.h"
#include "Position.h"
#include "Vector3.h"
#include "TilePosition.h"
#include "TileOffset.h"
#include "SharedConfig.h"
#include "Log.h"

namespace AM
{
namespace Client
{
SDL_FRect ClientTransforms::entityToScreenExtent(
    const Position& position, const Vector3& idleSouthBottomCenter,
    const Vector3& alignmentOffset, const SpriteRenderData& renderData,
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

    // Offset the sprite to line up with IdleSouth's modelBounds bottom center.
    SDL_FPoint idleSouthBottomCenterScreen{
        Transforms::worldToScreen(idleSouthBottomCenter, camera.zoomFactor)};
    screenPoint.x -= idleSouthBottomCenterScreen.x;
    screenPoint.y -= idleSouthBottomCenterScreen.y;

    // Offset the sprite to account for the alignment offset.
    SDL_FPoint alignmentOffsetScreen{
        Transforms::worldToScreen(alignmentOffset, camera.zoomFactor)};
    screenPoint.x += alignmentOffsetScreen.x;
    screenPoint.y += alignmentOffsetScreen.y;

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
