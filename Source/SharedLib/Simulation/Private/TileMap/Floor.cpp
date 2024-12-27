#include "Floor.h"
#include "TilePosition.h"
#include "Position.h"
#include "SharedConfig.h"
#include "AMAssert.h"

namespace AM
{
static constexpr float WIDTH{SharedConfig::TILE_WORLD_WIDTH};

/** World-space bounding boxes for each terrain height. */
static constexpr BoundingBox FLOOR_BOX{{0, 0, 0}, {WIDTH, WIDTH, 0}};

BoundingBox Floor::calcWorldBounds(const TilePosition& tilePosition)
{
    // Move the bounds to the tile's origin.
    Vector3 tileOrigin{tilePosition.getOriginPoint()};
    return FLOOR_BOX.translateBy(tileOrigin);
}

} // End namespace AM
