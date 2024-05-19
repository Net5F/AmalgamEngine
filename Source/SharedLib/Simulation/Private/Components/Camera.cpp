#include "Camera.h"
#include "Transforms.h"
#include "SharedConfig.h"

namespace AM
{

TileExtent Camera::getTileViewExtent(const TileExtent& mapTileExtent) const
{
    TileExtent tileViewExtent{viewBounds.asTileExtent()};

    // Clip the view to the tile map's bounds.
    tileViewExtent.intersectWith(mapTileExtent);

    return tileViewExtent;
}

} // End namespace AM
