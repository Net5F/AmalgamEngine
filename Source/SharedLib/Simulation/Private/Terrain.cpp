#include "Terrain.h"
#include "TilePosition.h"
#include "Position.h"
#include "SharedConfig.h"
#include "AMAssert.h"

namespace AM
{
static constexpr float WIDTH{SharedConfig::TILE_WORLD_WIDTH};
static constexpr float HEIGHT{SharedConfig::TILE_WORLD_HEIGHT};
static constexpr float ONE_THIRD_HEIGHT{SharedConfig::TILE_WORLD_HEIGHT / 3};
static constexpr float TWO_THIRD_HEIGHT{(SharedConfig::TILE_WORLD_HEIGHT / 3)
                                        * 2};

/** World-space bounding boxes for each terrain height. */
static constexpr std::array<BoundingBox, Terrain::Height::Count> TERRAIN_BOXES{
    {{0, WIDTH, 0, WIDTH, 0, 0},
     {0, WIDTH, 0, WIDTH, 0, ONE_THIRD_HEIGHT},
     {0, WIDTH, 0, WIDTH, 0, TWO_THIRD_HEIGHT},
     {0, WIDTH, 0, WIDTH, 0, HEIGHT}}};

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

    // Move the bounds to the tile position.
    Position tilePos{tilePosition.getOriginPosition()};
    bounds.minX += tilePos.x;
    bounds.maxX += tilePos.x;
    bounds.minY += tilePos.y;
    bounds.maxY += tilePos.y;
    bounds.minZ += tilePos.z;
    bounds.maxZ += tilePos.z;

    // Raise the bounds based on the given start height.
    bounds.minZ += HEIGHT_VALUES[startHeight];
    bounds.maxZ += HEIGHT_VALUES[startHeight];

    return bounds;
}

} // End namespace AM
