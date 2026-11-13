#include "NavGrid.h"
#include "CollisionLocator.h"
#include "AMMath.h"
#include "AMAssert.h"
#include "Log.h"
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
, chunks{}
, version{0}
, chunkVolumes{}
, columnVolumeIndices(SharedConfig::CHUNK_TILE_COUNT)
, chunkColumnOffsets{}
, chunkSurfaces{}
, candidateHeights{}
, westIntervals{}
, northIntervals{}
, dirtyChunkIndices{}
{
    // Track changes to the volumes that affect navigation, so that update()
    // knows which chunks to rebuild.
    collisionLocator.setTrackedChangeLayers(COLLISION_MASK);
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
    AM_ASSERT((tileExtent.zLength * 3) <= SDL_MAX_UINT16,
              "Nav grid extent is too tall for surface heights to fit.");

    gridTileExtent = tileExtent;
    chunkXLength = tileExtent.xLength / CHUNK_WIDTH;
    chunkYLength = tileExtent.yLength / CHUNK_WIDTH;

    chunks.clear();
    chunks.resize(static_cast<std::size_t>(chunkXLength * chunkYLength));
    version++;
}

void NavGrid::build()
{
    ZoneScoped;

    // We're rebuilding everything, so any tracked changes are irrelevant.
    collisionLocator.clearTrackedChanges();

    for (int chunkIndex{0}; chunkIndex < static_cast<int>(chunks.size());
         ++chunkIndex) {
        buildChunk(chunkIndex);
    }

    version++;
}

void NavGrid::update()
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    const std::vector<BoundingBox>& changes{
        collisionLocator.getTrackedChanges()};
    if (changes.empty()) {
        return;
    }
    ZoneScoped;

    // Find the chunks that each change may affect.
    // Note: A column's surfaces only depend on the volumes within it, so we
    //       only need to rebuild the chunks that the changes overlap.
    // Note: Like addVolumeToColumns(), this includes columns that are only
    //       touched.
    dirtyChunkIndices.clear();
    for (const BoundingBox& change : changes) {
        int minX{
            std::max(static_cast<int>(std::floor(change.min.x / TILE_WIDTH)),
                     gridTileExtent.x)};
        int maxX{
            std::min(static_cast<int>(std::floor(change.max.x / TILE_WIDTH)),
                     gridTileExtent.xMax())};
        int minY{
            std::max(static_cast<int>(std::floor(change.min.y / TILE_WIDTH)),
                     gridTileExtent.y)};
        int maxY{
            std::min(static_cast<int>(std::floor(change.max.y / TILE_WIDTH)),
                     gridTileExtent.yMax())};
        if ((minX > maxX) || (minY > maxY)) {
            continue;
        }

        int minChunkX{(minX - gridTileExtent.x) / CHUNK_WIDTH};
        int maxChunkX{(maxX - gridTileExtent.x) / CHUNK_WIDTH};
        int minChunkY{(minY - gridTileExtent.y) / CHUNK_WIDTH};
        int maxChunkY{(maxY - gridTileExtent.y) / CHUNK_WIDTH};
        for (int chunkY{minChunkY}; chunkY <= maxChunkY; ++chunkY) {
            for (int chunkX{minChunkX}; chunkX <= maxChunkX; ++chunkX) {
                dirtyChunkIndices.push_back((chunkY * chunkXLength) + chunkX);
            }
        }
    }
    collisionLocator.clearTrackedChanges();

    // Rebuild each dirty chunk.
    std::ranges::sort(dirtyChunkIndices);
    auto duplicates{std::ranges::unique(dirtyChunkIndices)};
    dirtyChunkIndices.erase(duplicates.begin(), duplicates.end());
    for (int chunkIndex : dirtyChunkIndices) {
        buildChunk(chunkIndex);
    }

    if (!(dirtyChunkIndices.empty())) {
        version++;
    }
}

std::span<const NavSurface> NavGrid::getSurfaces(int x, int y) const
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    if ((x < gridTileExtent.x) || (x > gridTileExtent.xMax())
        || (y < gridTileExtent.y) || (y > gridTileExtent.yMax())) {
        return {};
    }

    const Chunk& chunk{chunks[getChunkIndex(x, y)]};
    if (chunk.columnOffsets.empty()) {
        return {};
    }

    int columnIndex{(((y - gridTileExtent.y) % CHUNK_WIDTH) * CHUNK_WIDTH)
                    + ((x - gridTileExtent.x) % CHUNK_WIDTH)};
    Uint16 start{chunk.columnOffsets[columnIndex]};
    Uint16 end{chunk.columnOffsets[columnIndex + 1]};
    return {chunk.surfaces.data() + start,
            static_cast<std::size_t>(end - start)};
}

const NavSurface* NavGrid::findSurface(int x, int y, int height) const
{
    for (const NavSurface& surface : getSurfaces(x, y)) {
        if (surface.height == height) {
            return &surface;
        }
        else if (surface.height > height) {
            break;
        }
    }

    return nullptr;
}

const TileExtent& NavGrid::getTileExtent() const
{
    return gridTileExtent;
}

Uint32 NavGrid::getVersion() const
{
    return version;
}

float NavGrid::getWorldZ(int height) const
{
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    return (gridTileExtent.z * TILE_HEIGHT) + (height * THIRD_HEIGHT);
}

void NavGrid::buildChunk(int chunkIndex)
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    int chunkOriginX{gridTileExtent.x
                     + ((chunkIndex % chunkXLength) * CHUNK_WIDTH)};
    int chunkOriginY{gridTileExtent.y
                     + ((chunkIndex / chunkXLength) * CHUNK_WIDTH)};

    // Copy this chunk's volumes out of the locator (the pointers that it
    // returns aren't stable).
    TileExtent chunkExtent{chunkOriginX, chunkOriginY, gridTileExtent.z,
                           CHUNK_WIDTH,  CHUNK_WIDTH,  gridTileExtent.zLength};
    chunkVolumes.clear();
    for (const CollisionLocator::CollisionInfo* collisionInfo :
         collisionLocator.getCollisions(chunkExtent, COLLISION_MASK)) {
        chunkVolumes.emplace_back(collisionInfo->collisionVolume,
                                  collisionInfo->collisionLayers);
    }

    // Sort the volumes into the columns that they may intersect.
    for (std::vector<Uint32>& volumeIndices : columnVolumeIndices) {
        volumeIndices.clear();
    }
    for (Uint32 i{0}; i < chunkVolumes.size(); ++i) {
        addVolumeToColumns(i, chunkOriginX, chunkOriginY);
    }

    // Build each column's surfaces.
    // Note: Column offsets are Uint16, so a chunk can hold at most
    //       SDL_MAX_UINT16 surfaces. Real chunks come nowhere close, but if
    //       one does, we drop the surfaces that don't fit.
    chunkColumnOffsets.clear();
    chunkSurfaces.clear();
    bool isFull{false};
    for (int y{chunkOriginY}; y < (chunkOriginY + CHUNK_WIDTH); ++y) {
        for (int x{chunkOriginX}; x < (chunkOriginX + CHUNK_WIDTH); ++x) {
            std::size_t columnStart{chunkSurfaces.size()};
            chunkColumnOffsets.push_back(static_cast<Uint16>(columnStart));
            if (isFull) {
                continue;
            }

            int columnIndex{((y - chunkOriginY) * CHUNK_WIDTH)
                            + (x - chunkOriginX)};
            buildColumn(x, y, columnVolumeIndices[columnIndex]);

            // If this column didn't fit, drop it and skip the rest.
            if (chunkSurfaces.size() > SDL_MAX_UINT16) {
                chunkSurfaces.resize(columnStart);
                isFull = true;
            }
        }
    }
    chunkColumnOffsets.push_back(static_cast<Uint16>(chunkSurfaces.size()));
    if (isFull) {
        LOG_ERROR("Nav chunk at (%d, %d) has too many surfaces. Some were "
                  "dropped.",
                  chunkOriginX, chunkOriginY);
    }

    // Save the surfaces. If there aren't any, don't allocate anything.
    Chunk& chunk{chunks[chunkIndex]};
    if (chunkSurfaces.empty()) {
        chunk = {};
    }
    else {
        chunk.columnOffsets.assign(chunkColumnOffsets.begin(),
                                   chunkColumnOffsets.end());
        chunk.surfaces.assign(chunkSurfaces.begin(), chunkSurfaces.end());
    }
}

void NavGrid::addVolumeToColumns(Uint32 volumeIndex, int chunkOriginX,
                                 int chunkOriginY)
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    // Calc the range of columns that the volume may intersect, clipped to the
    // current chunk.
    // Note: This range includes columns that the volume only touches. That's
    //       fine, since surfaces check for actual intersections.
    const BoundingBox& bounds{chunkVolumes[volumeIndex].bounds};
    int minX{std::max(static_cast<int>(std::floor(bounds.min.x / TILE_WIDTH)),
                      chunkOriginX)};
    int maxX{std::min(static_cast<int>(std::floor(bounds.max.x / TILE_WIDTH)),
                      chunkOriginX + CHUNK_WIDTH - 1)};
    int minY{std::max(static_cast<int>(std::floor(bounds.min.y / TILE_WIDTH)),
                      chunkOriginY)};
    int maxY{std::min(static_cast<int>(std::floor(bounds.max.y / TILE_WIDTH)),
                      chunkOriginY + CHUNK_WIDTH - 1)};

    for (int y{minY}; y <= maxY; ++y) {
        for (int x{minX}; x <= maxX; ++x) {
            int columnIndex{((y - chunkOriginY) * CHUNK_WIDTH)
                            + (x - chunkOriginX)};
            columnVolumeIndices[columnIndex].push_back(volumeIndex);
        }
    }
}

void NavGrid::buildColumn(int x, int y,
                          const std::vector<Uint32>& volumeIndices)
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};
    static constexpr float EPSILON{Math::WORLD_EPSILON};

    // Find the candidate surfaces: the tops of any terrain that covers this
    // tile and ends within the grid.
    const float tileMinX{x * TILE_WIDTH};
    const float tileMinY{y * TILE_WIDTH};
    const float gridMinZ{gridTileExtent.z * TILE_HEIGHT};
    const float gridMaxZ{(gridTileExtent.zMax() + 1) * TILE_HEIGHT};
    candidateHeights.clear();
    for (Uint32 volumeIndex : volumeIndices) {
        const Volume& volume{chunkVolumes[volumeIndex]};
        float top{volume.bounds.max.z};
        if (!(volume.collisionLayers & CollisionLayerType::TerrainWall)
            || !coversTile(volume.bounds, tileMinX, tileMinY)
            || (top < (gridMinZ - EPSILON)) || (top >= (gridMaxZ - EPSILON))) {
            continue;
        }

        candidateHeights.push_back(static_cast<int>(
            std::floor((top - gridMinZ + EPSILON) / THIRD_HEIGHT)));
    }
    std::ranges::sort(candidateHeights);
    auto duplicates{std::ranges::unique(candidateHeights)};
    candidateHeights.erase(duplicates.begin(), duplicates.end());

    // Add each candidate that has room to stand on it.
    for (int height : candidateHeights) {
        NavSurface surface{};
        if (calcSurface(x, y, height, volumeIndices, surface)) {
            chunkSurfaces.push_back(surface);
        }
    }
}

bool NavGrid::calcSurface(int x, int y, int height,
                          const std::vector<Uint32>& volumeIndices,
                          NavSurface& surface)
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
    const float surfaceZ{getWorldZ(height)};
    const float gridMaxZ{(gridTileExtent.zMax() + 1) * TILE_HEIGHT};

    // Find the headroom and the covered portions of the edges.
    // Note: The top of the grid acts as a ceiling, since entities can't move
    //       outside of it.
    float ceilingZ{std::min(
        surfaceZ + (NavSurface::MAX_HEADROOM * THIRD_HEIGHT), gridMaxZ)};
    westIntervals.clear();
    northIntervals.clear();
    for (Uint32 volumeIndex : volumeIndices) {
        const BoundingBox& bounds{chunkVolumes[volumeIndex].bounds};

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
        // Note: We only check one tile's height above the surface. Checking
        //       higher would block doorways that have a wall above them.
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

    // If there's no room to stand, there's no surface.
    float headroom{std::floor((ceilingZ - surfaceZ + EPSILON) / THIRD_HEIGHT)};
    if (headroom < 1) {
        return false;
    }

    surface.height = static_cast<Uint16>(height);
    surface.headroom = static_cast<Uint8>(
        std::min(headroom, static_cast<float>(NavSurface::MAX_HEADROOM)));
    surface.setWestBlocked(calcCoveredLength(westIntervals)
                           > EDGE_BLOCKED_LENGTH);
    surface.setNorthBlocked(calcCoveredLength(northIntervals)
                            > EDGE_BLOCKED_LENGTH);
    return true;
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

int NavGrid::getChunkIndex(int x, int y) const
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    return ((((y - gridTileExtent.y) / CHUNK_WIDTH) * chunkXLength)
            + ((x - gridTileExtent.x) / CHUNK_WIDTH));
}

} // End namespace AM
