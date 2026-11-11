#include "NavGrid.h"
#include "CollisionLocator.h"
#include "TilePosition.h"
#include "AMMath.h"
#include "AMAssert.h"
#include "tracy/Tracy.hpp"
#include <algorithm>
#include <cmath>

namespace AM
{

NavGrid::NavGrid(CollisionLocator& inCollisionLocator)
: collisionLocator{inCollisionLocator}
, gridTileExtent{}
, chunkXLength{0}
, chunkYLength{0}
, nodes{}
, columnOriginX{0}
, columnOriginY{0}
, columnVolumes{}
, tileVolumeIndices{}
, nodeVolumeIndices{}
, westIntervals{}
, northIntervals{}
{
}

void NavGrid::setGridSize(const TileExtent& tileExtent)
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    AM_ASSERT(((tileExtent.x % CHUNK_WIDTH) == 0)
                  && ((tileExtent.y % CHUNK_WIDTH) == 0)
                  && ((tileExtent.xLength % CHUNK_WIDTH) == 0)
                  && ((tileExtent.yLength % CHUNK_WIDTH) == 0),
              "Nav grid extent must be aligned to chunk boundaries.");

    gridTileExtent = tileExtent;
    chunkXLength = tileExtent.xLength / CHUNK_WIDTH;
    chunkYLength = tileExtent.yLength / CHUNK_WIDTH;

    nodes.assign(tileExtent.size(), NavNode{});
    tileVolumeIndices.resize(SharedConfig::CHUNK_TILE_COUNT
                             * tileExtent.zLength);
}

void NavGrid::build()
{
    ZoneScoped;

    std::ranges::fill(nodes, NavNode{});

    // Build each column of chunks.
    for (int chunkY{0}; chunkY < chunkYLength; ++chunkY) {
        for (int chunkX{0}; chunkX < chunkXLength; ++chunkX) {
            buildChunkColumn(
                gridTileExtent.x
                    + (chunkX * static_cast<int>(SharedConfig::CHUNK_WIDTH)),
                gridTileExtent.y
                    + (chunkY * static_cast<int>(SharedConfig::CHUNK_WIDTH)));
        }
    }

    // Clearance depends on neighboring nodes, so it's built after all nodes
    // are otherwise complete.
    buildClearance();
}

NavNode NavGrid::getNode(const TilePosition& tilePosition) const
{
    if (!(gridTileExtent.contains(tilePosition))) {
        return {};
    }

    return nodes[linearizeIndex(tilePosition.x, tilePosition.y,
                                tilePosition.z)];
}

const TileExtent& NavGrid::getTileExtent() const
{
    return gridTileExtent;
}

std::size_t NavGrid::linearizeIndex(int x, int y, int z) const
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    // Translate the given position from actual-space to positive-space.
    int positiveX{x - gridTileExtent.x};
    int positiveY{y - gridTileExtent.y};
    int positiveZ{z - gridTileExtent.z};

    int chunkIndex{(((positiveZ * chunkYLength) + (positiveY / CHUNK_WIDTH))
                    * chunkXLength)
                   + (positiveX / CHUNK_WIDTH)};
    int tileIndex{((positiveY % CHUNK_WIDTH) * CHUNK_WIDTH)
                  + (positiveX % CHUNK_WIDTH)};

    return (static_cast<std::size_t>(chunkIndex)
            * SharedConfig::CHUNK_TILE_COUNT)
           + static_cast<std::size_t>(tileIndex);
}

std::size_t NavGrid::linearizeColumnIndex(int x, int y, int z) const
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    static constexpr int CHUNK_TILE_COUNT{
        static_cast<int>(SharedConfig::CHUNK_TILE_COUNT)};

    return static_cast<std::size_t>(((z - gridTileExtent.z) * CHUNK_TILE_COUNT)
                                    + ((y - columnOriginY) * CHUNK_WIDTH)
                                    + (x - columnOriginX));
}

void NavGrid::buildChunkColumn(int originX, int originY)
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    columnOriginX = originX;
    columnOriginY = originY;

    // Copy this column's volumes out of the locator (the pointers that it
    // returns aren't stable).
    TileExtent columnExtent{originX,     originY,     gridTileExtent.z,
                            CHUNK_WIDTH, CHUNK_WIDTH, gridTileExtent.zLength};
    columnVolumes.clear();
    for (const CollisionLocator::CollisionInfo* collisionInfo :
         collisionLocator.getCollisions(columnExtent, COLLISION_MASK)) {
        columnVolumes.emplace_back(collisionInfo->collisionVolume,
                                   collisionInfo->collisionLayers);
    }

    // Sort the volumes into the tiles that they may intersect.
    for (std::vector<Uint32>& volumeIndices : tileVolumeIndices) {
        volumeIndices.clear();
    }
    for (Uint32 i{0}; i < columnVolumes.size(); ++i) {
        addVolumeToTiles(i);
    }

    // Calculate each node in the column.
    for (int z{gridTileExtent.z}; z <= gridTileExtent.zMax(); ++z) {
        for (int y{originY}; y < (originY + CHUNK_WIDTH); ++y) {
            for (int x{originX}; x < (originX + CHUNK_WIDTH); ++x) {
                nodes[linearizeIndex(x, y, z)] = calcNode(x, y, z);
            }
        }
    }
}

void NavGrid::addVolumeToTiles(Uint32 volumeIndex)
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    // Calc the range of tiles that the volume may intersect, clipped to the
    // current column.
    // Note: This range includes tiles that the volume only touches. That's
    //       fine, since nodes check for actual intersections.
    const BoundingBox& bounds{columnVolumes[volumeIndex].bounds};
    int minX{std::max(static_cast<int>(std::floor(bounds.min.x / TILE_WIDTH)),
                      columnOriginX)};
    int maxX{std::min(static_cast<int>(std::floor(bounds.max.x / TILE_WIDTH)),
                      columnOriginX + CHUNK_WIDTH - 1)};
    int minY{std::max(static_cast<int>(std::floor(bounds.min.y / TILE_WIDTH)),
                      columnOriginY)};
    int maxY{std::min(static_cast<int>(std::floor(bounds.max.y / TILE_WIDTH)),
                      columnOriginY + CHUNK_WIDTH - 1)};
    int minZ{std::max(static_cast<int>(std::floor(bounds.min.z / TILE_HEIGHT)),
                      gridTileExtent.z)};
    int maxZ{std::min(static_cast<int>(std::floor(bounds.max.z / TILE_HEIGHT)),
                      gridTileExtent.zMax())};

    for (int z{minZ}; z <= maxZ; ++z) {
        for (int y{minY}; y <= maxY; ++y) {
            for (int x{minX}; x <= maxX; ++x) {
                tileVolumeIndices[linearizeColumnIndex(x, y, z)].push_back(
                    volumeIndex);
            }
        }
    }
}

NavNode NavGrid::calcNode(int x, int y, int z)
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};
    static constexpr float EPSILON{Math::WORLD_EPSILON};

    const float tileMinX{x * TILE_WIDTH};
    const float tileMaxX{tileMinX + TILE_WIDTH};
    const float tileMinY{y * TILE_WIDTH};
    const float tileMaxY{tileMinY + TILE_WIDTH};
    const float tileMinZ{z * TILE_HEIGHT};

    gatherNodeVolumes(x, y, z);

    // Find the surface: the highest top of any terrain that covers this tile
    // and ends within it.
    bool hasSurface{false};
    float surfaceZ{0};
    for (Uint32 volumeIndex : nodeVolumeIndices) {
        const Volume& volume{columnVolumes[volumeIndex]};
        float top{volume.bounds.max.z};
        if (!(volume.collisionLayers & CollisionLayerType::TerrainWall)
            || !coversTile(volume.bounds, tileMinX, tileMinY)
            || (top < (tileMinZ - EPSILON))
            || (top >= (tileMinZ + TILE_HEIGHT - EPSILON))) {
            continue;
        }

        if (!hasSurface || (top > surfaceZ)) {
            surfaceZ = top;
            hasSurface = true;
        }
    }
    if (!hasSurface) {
        return {};
    }

    // Find the headroom and the covered portions of the edges.
    float ceilingZ{surfaceZ + (NavNode::MAX_HEADROOM * THIRD_HEIGHT)};
    westIntervals.clear();
    northIntervals.clear();
    for (Uint32 volumeIndex : nodeVolumeIndices) {
        const BoundingBox& bounds{columnVolumes[volumeIndex].bounds};

        // Skip any volumes that don't overlap this tile's interior, or that
        // are at or below the surface.
        if ((bounds.max.x <= (tileMinX + EPSILON))
            || (bounds.min.x >= (tileMaxX - EPSILON))
            || (bounds.max.y <= (tileMinY + EPSILON))
            || (bounds.min.y >= (tileMaxY - EPSILON))
            || (bounds.max.z <= (surfaceZ + EPSILON))) {
            continue;
        }

        // If this volume fits within an edge band, add it to that edge (if
        // it's low enough to get in the way).
        float clippedMaxX{std::min(bounds.max.x, tileMaxX)};
        float clippedMaxY{std::min(bounds.max.y, tileMaxY)};
        bool inWestBand{clippedMaxX <= (tileMinX + EDGE_BAND_WIDTH)};
        bool inNorthBand{clippedMaxY <= (tileMinY + EDGE_BAND_WIDTH)};
        if (inWestBand || inNorthBand) {
            if (bounds.min.z < (surfaceZ + TILE_HEIGHT)) {
                if (inWestBand) {
                    westIntervals.emplace_back(std::max(bounds.min.y, tileMinY),
                                               clippedMaxY);
                }
                if (inNorthBand) {
                    northIntervals.emplace_back(
                        std::max(bounds.min.x, tileMinX), clippedMaxX);
                }
            }
            continue;
        }

        // This volume is an obstacle above the surface. Lower the ceiling to
        // meet it.
        ceilingZ = std::min(ceilingZ, std::max(bounds.min.z, surfaceZ));
    }

    // If there's no room to stand, this node isn't standable.
    Uint8 headroom{static_cast<Uint8>(
        std::floor((ceilingZ - surfaceZ + EPSILON) / THIRD_HEIGHT))};
    headroom = std::min(headroom, NavNode::MAX_HEADROOM);
    if (headroom == 0) {
        return {};
    }

    Uint8 surfaceHeight{static_cast<Uint8>(
        std::floor((surfaceZ - tileMinZ + EPSILON) / THIRD_HEIGHT))};
    surfaceHeight = std::min(surfaceHeight, NavNode::MAX_SURFACE_HEIGHT);

    NavNode node{};
    node.setSurfaceHeight(surfaceHeight);
    node.setClearance(1);
    node.setHeadroom(headroom);
    node.setWestBlocked(calcCoveredLength(westIntervals) > EDGE_BLOCKED_LENGTH);
    node.setNorthBlocked(calcCoveredLength(northIntervals)
                         > EDGE_BLOCKED_LENGTH);
    return node;
}

void NavGrid::gatherNodeVolumes(int x, int y, int z)
{
    // A node's surface may come from the tile below it, and its headroom may
    // extend up to 2 tiles above it.
    nodeVolumeIndices.clear();
    int minZ{std::max(z - 1, gridTileExtent.z)};
    int maxZ{std::min(z + 2, gridTileExtent.zMax())};
    for (int volumeZ{minZ}; volumeZ <= maxZ; ++volumeZ) {
        const std::vector<Uint32>& volumeIndices{
            tileVolumeIndices[linearizeColumnIndex(x, y, volumeZ)]};
        nodeVolumeIndices.insert(nodeVolumeIndices.end(), volumeIndices.begin(),
                                 volumeIndices.end());
    }

    // Note: Volumes that span multiple tiles will be duplicated. That's fine,
    //       since duplicates don't change the result.
}

bool NavGrid::coversTile(const BoundingBox& bounds, float tileMinX,
                         float tileMinY) const
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float EPSILON{Math::WORLD_EPSILON};

    return (bounds.min.x <= (tileMinX + EPSILON))
           && (bounds.max.x >= (tileMinX + TILE_WIDTH - EPSILON))
           && (bounds.min.y <= (tileMinY + EPSILON))
           && (bounds.max.y >= (tileMinY + TILE_WIDTH - EPSILON));
}

float NavGrid::calcCoveredLength(
    std::vector<std::pair<float, float>>& intervals)
{
    if (intervals.empty()) {
        return 0;
    }

    // Merge any overlapping intervals and sum their lengths.
    std::ranges::sort(intervals);
    float coveredLength{0};
    auto [currentStart, currentEnd]{intervals[0]};
    for (const auto& [start, end] : intervals) {
        if (start > currentEnd) {
            coveredLength += (currentEnd - currentStart);
            currentStart = start;
            currentEnd = end;
        }
        else {
            currentEnd = std::max(currentEnd, end);
        }
    }
    coveredLength += (currentEnd - currentStart);

    return coveredLength;
}

void NavGrid::buildClearance()
{
    // Clearance builds on the nodes to the east, south, and southeast, so we
    // iterate in reverse.
    for (int z{gridTileExtent.z}; z <= gridTileExtent.zMax(); ++z) {
        for (int y{gridTileExtent.yMax()}; y >= gridTileExtent.y; --y) {
            for (int x{gridTileExtent.xMax()}; x >= gridTileExtent.x; --x) {
                NavNode& node{nodes[linearizeIndex(x, y, z)]};
                if (node.isStandable()) {
                    node.setClearance(calcClearance(x, y, z, node));
                }
            }
        }
    }
}

Uint8 NavGrid::calcClearance(int x, int y, int z, const NavNode& node) const
{
    // If this node is on the east or south edge of the grid, nothing can be
    // added to its square.
    if ((x == gridTileExtent.xMax()) || (y == gridTileExtent.yMax())) {
        return 1;
    }

    // To grow a square, the east, south, and southeast nodes must be
    // standable at the same height, with no blocked edges between them.
    const NavNode& eastNode{nodes[linearizeIndex(x + 1, y, z)]};
    const NavNode& southNode{nodes[linearizeIndex(x, y + 1, z)]};
    const NavNode& southEastNode{nodes[linearizeIndex(x + 1, y + 1, z)]};
    Uint8 surfaceHeight{node.getSurfaceHeight()};
    if (!(eastNode.isStandable()) || !(southNode.isStandable())
        || !(southEastNode.isStandable())
        || (eastNode.getSurfaceHeight() != surfaceHeight)
        || (southNode.getSurfaceHeight() != surfaceHeight)
        || (southEastNode.getSurfaceHeight() != surfaceHeight)
        || eastNode.isWestBlocked() || southNode.isNorthBlocked()
        || southEastNode.isWestBlocked() || southEastNode.isNorthBlocked()) {
        return 1;
    }

    // Grow the smallest of the neighboring squares.
    Uint8 neighborClearance{
        std::min({eastNode.getClearance(), southNode.getClearance(),
                  southEastNode.getClearance()})};
    return std::min(static_cast<Uint8>(neighborClearance + 1),
                    NavNode::MAX_CLEARANCE);
}

} // End namespace AM
