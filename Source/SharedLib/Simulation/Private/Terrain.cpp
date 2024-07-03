#include "Terrain.h"
#include "TilePosition.h"
#include "Position.h"
#include "SharedConfig.h"
#include "AMAssert.h"

namespace AM
{
static constexpr float WIDTH{SharedConfig::TILE_WORLD_WIDTH};
static constexpr float HEIGHT{SharedConfig::TILE_WORLD_HEIGHT};
static constexpr float HALF_WIDTH{WIDTH / 2};
static constexpr float HALF_HEIGHT{HEIGHT / 2};
static constexpr float ONE_THIRD_HEIGHT{HEIGHT / 3};
static constexpr float TWO_THIRD_HEIGHT{(HEIGHT / 3) * 2};
static constexpr float HALF_ONE_THIRD_HEIGHT{ONE_THIRD_HEIGHT / 2};
static constexpr float HALF_TWO_THIRD_HEIGHT{TWO_THIRD_HEIGHT / 2};

/** World-space bounding boxes for each terrain height. */
static constexpr std::array<BoundingBox, Terrain::Height::Count> TERRAIN_BOXES{
    {{{HALF_WIDTH, HALF_WIDTH, 0}, {HALF_WIDTH, HALF_WIDTH, 0}},
     {{HALF_WIDTH, HALF_WIDTH, HALF_ONE_THIRD_HEIGHT},
      {HALF_WIDTH, HALF_WIDTH, HALF_ONE_THIRD_HEIGHT}},
     {{HALF_WIDTH, HALF_WIDTH, HALF_TWO_THIRD_HEIGHT},
      {HALF_WIDTH, HALF_WIDTH, HALF_TWO_THIRD_HEIGHT}},
     {{HALF_WIDTH, HALF_WIDTH, HALF_HEIGHT},
      {HALF_WIDTH, HALF_WIDTH, HALF_HEIGHT}}}};

/** World-space height offsets for each height value. */
static constexpr std::array<float, Terrain::Height::Count> HEIGHT_VALUES{
    0,
    ONE_THIRD_HEIGHT,
    TWO_THIRD_HEIGHT,
    HEIGHT
};

Terrain::Height Terrain::getHeight(Value value)
{
    Height height{static_cast<Height>(value >> 4)};
    AM_ASSERT(height < Height::Count, "Invalid terrain height.");

    return height;
}

Terrain::Height Terrain::getStartHeight(Value value)
{
    Height startHeight{static_cast<Height>(value & START_HEIGHT_MASK)};
    AM_ASSERT(startHeight < Height::Count, "Invalid terrain start height.");

    return startHeight;
}

Terrain::Height Terrain::getTotalHeight(Value value)
{
    auto [height, startHeight]{getInfo(value)};
    return static_cast<Terrain::Height>(height + startHeight);
}

Terrain::InfoReturn Terrain::getInfo(Value value)
{
    Height height{static_cast<Height>(value >> 4)};
    Height startHeight{static_cast<Height>(value & START_HEIGHT_MASK)};
    AM_ASSERT(height < Height::Count, "Invalid terrain height.");
    AM_ASSERT(startHeight < Height::Count, "Invalid terrain start height.");

    return {height, startHeight};
}

Terrain::Value Terrain::toValue(Height height, Height startHeight)
{
    return (height << 4) | startHeight;
}

float Terrain::getHeightWorldValue(Height height)
{
    return HEIGHT_VALUES[height];
}

BoundingBox Terrain::calcWorldBounds(const TilePosition& tilePosition,
                                     Value value)
{
    auto [height, startHeight]{getInfo(value)};

    // Get the appropriate bounds for the given tile height.
    BoundingBox bounds{TERRAIN_BOXES[height]};

    // Move the bounds to the tile's origin.
    bounds.moveMinimumTo(tilePosition.getOriginPoint());

    // Raise the bounds based on the given start height.
    bounds.center.z += HEIGHT_VALUES[startHeight];

    return bounds;
}

} // End namespace AM
