#include "Camera.h"
#include "Transforms.h"
#include "SharedConfig.h"

namespace AM
{

TileExtent Camera::getTileViewExtent(const TileExtent& tileMapExtent) const
{
    // Find the world position that this camera is centered on.
    SDL_FPoint centerPoint{(extent.w / 2), (extent.h / 2)};
    Position centerPosition{Transforms::screenToWorld(centerPoint, *this)};

    // Issues with float precision can cause flickering tiles. Round to the
    // nearest whole number to avoid this.
    centerPosition.x = std::round(centerPosition.x);
    centerPosition.y = std::round(centerPosition.y);

    // Find the lowest x/y tile indices that this camera can see.
    TileExtent tileViewExtent{};
    tileViewExtent.x = static_cast<int>(
        std::floor((centerPosition.x - SharedConfig::VIEW_RADIUS)
                   / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)));
    tileViewExtent.y = static_cast<int>(
        std::floor((centerPosition.y - SharedConfig::VIEW_RADIUS)
                   / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)));

    // Calc how far the player's view extends.
    // Note: We add 1 to the view radius to keep all sides even, since the
    //       player occupies a tile.
    tileViewExtent.xLength
        = static_cast<int>(std::ceil(((SharedConfig::VIEW_RADIUS * 2) + 1)
                                     / SharedConfig::TILE_WORLD_WIDTH));
    tileViewExtent.yLength = tileViewExtent.xLength;

    // Clip the view to the world bounds.
    tileViewExtent.intersectWith(tileMapExtent);

    return tileViewExtent;
}

} // End namespace AM
