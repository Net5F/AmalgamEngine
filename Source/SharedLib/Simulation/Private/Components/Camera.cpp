#include "Camera.h"
#include "Transforms.h"
#include "SharedConfig.h"

namespace AM
{

TileExtent Camera::getTileViewExtent(const TileExtent& mapTileExtent) const
{
    TileExtent tileViewExtent(viewBounds);

    // Clip the view to the tile map's bounds.
    return tileViewExtent.intersectWith(mapTileExtent);
}

} // End namespace AM
