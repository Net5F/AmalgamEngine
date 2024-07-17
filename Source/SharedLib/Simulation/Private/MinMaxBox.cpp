#include "MinMaxBox.h"
#include "BoundingBox.h"
#include "SharedConfig.h"
#include <cmath>

namespace AM
{

MinMaxBox::MinMaxBox()
: min{}
, max{}
{
}

MinMaxBox::MinMaxBox(const Vector3& inMin, const Vector3& inMax)
: min{inMin}
, max{inMax}
{
}

MinMaxBox::MinMaxBox(const BoundingBox& box)
: min{(box.center.x - box.halfExtents.x),
      (box.center.y - box.halfExtents.y),
      (box.center.z - box.halfExtents.z)}
, max{(box.center.x + box.halfExtents.x),
      (box.center.y + box.halfExtents.y),
      (box.center.z + box.halfExtents.z)}
{
}

TileExtent MinMaxBox::asTileExtent() const
{
    static constexpr float TILE_WORLD_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_WORLD_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    TileExtent tileExtent{};
    tileExtent.x = static_cast<int>(std::floor(min.x / TILE_WORLD_WIDTH));
    tileExtent.y = static_cast<int>(std::floor(min.y / TILE_WORLD_WIDTH));
    tileExtent.z = static_cast<int>(std::floor(min.z / TILE_WORLD_HEIGHT));
    tileExtent.xLength
        = (static_cast<int>(std::ceil(max.x / TILE_WORLD_WIDTH))
           - tileExtent.x);
    tileExtent.yLength
        = (static_cast<int>(std::ceil(max.y / TILE_WORLD_WIDTH))
           - tileExtent.y);
    tileExtent.zLength
        = (static_cast<int>(std::ceil(max.z / TILE_WORLD_HEIGHT))
           - tileExtent.z);

    return tileExtent;
}

} // End namespace AM
