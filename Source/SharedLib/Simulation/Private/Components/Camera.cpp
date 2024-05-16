#include "Camera.h"
#include "Transforms.h"
#include "SharedConfig.h"

namespace AM
{

TileExtent Camera::getTileViewExtent(const TileExtent& mapTileExtent) const
{
    static constexpr int HALF_VIEW_WIDTH_TILES{
        static_cast<int>(SharedConfig::VIEW_RADIUS
                         / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH))};
    static constexpr int HALF_VIEW_HEIGHT_TILES{static_cast<int>(
        SharedConfig::VIEW_RADIUS
        / static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT))};

    // Find the lowest x/y tile indices that this camera can see.
    TilePosition centerTile{target.asTilePosition()};
    TileExtent tileViewExtent{};
    tileViewExtent.x = centerTile.x - HALF_VIEW_WIDTH_TILES;
    tileViewExtent.y = centerTile.y - HALF_VIEW_WIDTH_TILES;
    // Note: The camera can always see down to Z == 0.

    // Calc how far the player's view extends.
    // Note: We add 1 to the view radius to keep all sides even, since the
    //       player occupies a tile.
    tileViewExtent.xLength
        = static_cast<int>(std::ceil(((SharedConfig::VIEW_RADIUS * 2) + 1)
                                     / SharedConfig::TILE_WORLD_WIDTH));
    tileViewExtent.yLength = tileViewExtent.xLength;
    tileViewExtent.zLength
        = static_cast<int>(std::ceil(((SharedConfig::VIEW_RADIUS * 2) + 1)
                                     / SharedConfig::TILE_WORLD_HEIGHT));

    // Clip the view to the world bounds.
    tileViewExtent.intersectWith(mapTileExtent);

    return tileViewExtent;
}

} // End namespace AM
