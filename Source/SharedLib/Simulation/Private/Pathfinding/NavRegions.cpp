#include "NavRegions.h"
#include "NavGrid.h"
#include "AMAssert.h"
#include "tracy/Tracy.hpp"
#include <algorithm>
#include <array>

namespace AM
{

NavRegions::NavRegions(const NavGrid& inNavGrid)
: navGrid{inNavGrid}
, chunks{}
, regionBases{}
, columnSurfaces(SharedConfig::CHUNK_TILE_COUNT)
, columnStarts(SharedConfig::CHUNK_TILE_COUNT)
, surfaceColumns{}
, fillStack{}
, linkScratch{}
, linkChunkIndices{}
, visitedMarks{}
, searchID{0}
, frontier{}
{
}

void NavRegions::build()
{
    ZoneScoped;

    int chunkCount{navGrid.getChunkCount()};
    chunks.clear();
    chunks.resize(static_cast<std::size_t>(chunkCount));

    // Links depend on the local regions of neighboring chunks, so build all
    // local regions first.
    for (int chunkIndex{0}; chunkIndex < chunkCount; ++chunkIndex) {
        buildLocalRegions(chunkIndex);
    }
    for (int chunkIndex{0}; chunkIndex < chunkCount; ++chunkIndex) {
        buildLinks(chunkIndex);
    }

    calcRegionBases();
}

void NavRegions::update()
{
    const std::vector<int>& updatedChunks{navGrid.getUpdatedChunks()};
    if (updatedChunks.empty()) {
        return;
    }
    ZoneScoped;

    // Rebuild the updated chunks' local regions.
    for (int chunkIndex : updatedChunks) {
        buildLocalRegions(chunkIndex);
    }

    // Rebuild the links of the updated chunks and their neighbors (whose
    // links may point to the updated chunks' regions).
    const TileExtent& gridExtent{navGrid.getTileExtent()};
    int chunkXLength{gridExtent.xLength
                     / static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    int chunkYLength{gridExtent.yLength
                     / static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    linkChunkIndices.clear();
    for (int chunkIndex : updatedChunks) {
        int chunkX{chunkIndex % chunkXLength};
        int chunkY{chunkIndex / chunkXLength};
        linkChunkIndices.push_back(chunkIndex);
        if (chunkX > 0) {
            linkChunkIndices.push_back(chunkIndex - 1);
        }
        if (chunkX < (chunkXLength - 1)) {
            linkChunkIndices.push_back(chunkIndex + 1);
        }
        if (chunkY > 0) {
            linkChunkIndices.push_back(chunkIndex - chunkXLength);
        }
        if (chunkY < (chunkYLength - 1)) {
            linkChunkIndices.push_back(chunkIndex + chunkXLength);
        }
    }
    std::ranges::sort(linkChunkIndices);
    auto duplicates{std::ranges::unique(linkChunkIndices)};
    linkChunkIndices.erase(duplicates.begin(), duplicates.end());
    for (int chunkIndex : linkChunkIndices) {
        buildLinks(chunkIndex);
    }

    calcRegionBases();
}

bool NavRegions::supportsAgent(const NavAgentProfile& agent) const
{
    return (agent.footprintTiles == 1)
           && (agent.heightThirds
               >= SharedConfig::PATHFINDING_COMMON_HEIGHT_THIRDS)
           && (agent.climbThirds
               <= SharedConfig::PATHFINDING_COMMON_CLIMB_THIRDS)
           && (agent.maxDropThirds
               <= SharedConfig::PATHFINDING_COMMON_MAX_DROP_THIRDS);
}

bool NavRegions::isReachable(int startX, int startY, int startHeight, int goalX,
                             int goalY, int goalHeight)
{
    // Find each surface's region. If either isn't in one, we don't know.
    const NavSurface* startSurface{
        navGrid.findSurface(startX, startY, startHeight)};
    const NavSurface* goalSurface{
        navGrid.findSurface(goalX, goalY, goalHeight)};
    if (!startSurface || !goalSurface) {
        return true;
    }

    int startChunk{navGrid.getChunkIndex(startX, startY)};
    int goalChunk{navGrid.getChunkIndex(goalX, goalY)};
    Uint16 startRegion{
        chunks[startChunk].surfaceRegions[navGrid.getSurfaceIndex(
            startChunk, *startSurface)]};
    Uint16 goalRegion{
        chunks[goalChunk]
            .surfaceRegions[navGrid.getSurfaceIndex(goalChunk, *goalSurface)]};
    if ((startRegion == NO_REGION) || (goalRegion == NO_REGION)) {
        return true;
    }
    if ((startChunk == goalChunk) && (startRegion == goalRegion)) {
        return true;
    }

    // Search through the region links, breadth-first.
    searchID++;
    if (searchID == 0) {
        // The ID wrapped around, so old marks may match it. Reset them.
        std::ranges::fill(visitedMarks, 0);
        searchID = 1;
    }
    RegionLink goalLink{static_cast<Uint32>(goalChunk), goalRegion};
    frontier.clear();
    frontier.push_back({static_cast<Uint32>(startChunk), startRegion});
    visitedMarks[regionBases[startChunk] + startRegion] = searchID;
    for (std::size_t i{0}; i < frontier.size(); ++i) {
        RegionLink current{frontier[i]};
        const ChunkRegions& chunk{chunks[current.chunkIndex]};
        for (Uint32 linkIndex{chunk.linkOffsets[current.region]};
             linkIndex < chunk.linkOffsets[current.region + 1]; ++linkIndex) {
            const RegionLink& link{chunk.links[linkIndex]};
            if (link == goalLink) {
                return true;
            }

            Uint32& visitedMark{
                visitedMarks[regionBases[link.chunkIndex] + link.region]};
            if (visitedMark != searchID) {
                visitedMark = searchID;
                frontier.push_back(link);
            }
        }
    }

    return false;
}

void NavRegions::buildLocalRegions(int chunkIndex)
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    // Gather the chunk's surfaces, by column.
    auto [originX, originY]{getChunkOrigin(chunkIndex)};
    std::size_t surfaceCount{0};
    surfaceColumns.clear();
    for (int localY{0}; localY < CHUNK_WIDTH; ++localY) {
        for (int localX{0}; localX < CHUNK_WIDTH; ++localX) {
            int column{(localY * CHUNK_WIDTH) + localX};
            columnSurfaces[column]
                = navGrid.getSurfaces(originX + localX, originY + localY);
            columnStarts[column] = static_cast<Uint16>(surfaceCount);
            surfaceCount += columnSurfaces[column].size();
            surfaceColumns.insert(surfaceColumns.end(),
                                  columnSurfaces[column].size(),
                                  static_cast<Uint8>(column));
        }
    }

    ChunkRegions& chunk{chunks[chunkIndex]};
    chunk.surfaceRegions.assign(surfaceCount, NO_REGION);
    chunk.regionCount = 0;

    // Flood fill each region, through steps that work in both directions.
    for (std::size_t seedIndex{0}; seedIndex < surfaceCount; ++seedIndex) {
        int seedColumn{surfaceColumns[seedIndex]};
        const NavSurface& seedSurface{
            columnSurfaces[seedColumn][seedIndex - columnStarts[seedColumn]]};
        if ((chunk.surfaceRegions[seedIndex] != NO_REGION)
            || !isStandable(seedSurface)) {
            continue;
        }

        Uint16 region{chunk.regionCount};
        chunk.regionCount++;
        chunk.surfaceRegions[seedIndex] = region;
        fillStack.clear();
        fillStack.push_back(static_cast<Uint16>(seedIndex));
        while (!(fillStack.empty())) {
            Uint16 surfaceIndex{fillStack.back()};
            fillStack.pop_back();
            int column{surfaceColumns[surfaceIndex]};
            const NavSurface& surface{
                columnSurfaces[column][surfaceIndex - columnStarts[column]]};
            int localX{column % CHUNK_WIDTH};
            int localY{column / CHUNK_WIDTH};

            static constexpr std::array<std::pair<int, int>, 4> OFFSETS{
                {{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};
            for (auto [xOffset, yOffset] : OFFSETS) {
                int neighborX{localX + xOffset};
                int neighborY{localY + yOffset};
                if ((neighborX < 0) || (neighborX >= CHUNK_WIDTH)
                    || (neighborY < 0) || (neighborY >= CHUNK_WIDTH)) {
                    continue;
                }

                int neighborColumn{(neighborY * CHUNK_WIDTH) + neighborX};
                std::span<const NavSurface> neighborSurfaces{
                    columnSurfaces[neighborColumn]};
                for (std::size_t i{0}; i < neighborSurfaces.size(); ++i) {
                    std::size_t neighborIndex{columnStarts[neighborColumn] + i};
                    const NavSurface& neighborSurface{neighborSurfaces[i]};
                    if ((chunk.surfaceRegions[neighborIndex] == NO_REGION)
                        && canStep(surface, neighborSurface, xOffset, yOffset)
                        && canStep(neighborSurface, surface, -xOffset,
                                   -yOffset)) {
                        chunk.surfaceRegions[neighborIndex] = region;
                        fillStack.push_back(static_cast<Uint16>(neighborIndex));
                    }
                }
            }
        }
    }
}

void NavRegions::buildLinks(int chunkIndex)
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};
    static constexpr std::array<std::pair<int, int>, 4> OFFSETS{
        {{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};

    // Find every step from one of this chunk's regions to a different
    // region.
    auto [originX, originY]{getChunkOrigin(chunkIndex)};
    ChunkRegions& chunk{chunks[chunkIndex]};
    const TileExtent& gridExtent{navGrid.getTileExtent()};
    std::size_t surfaceIndex{0};
    linkScratch.clear();
    for (int y{originY}; y < (originY + CHUNK_WIDTH); ++y) {
        for (int x{originX}; x < (originX + CHUNK_WIDTH); ++x) {
            for (const NavSurface& surface : navGrid.getSurfaces(x, y)) {
                Uint16 region{chunk.surfaceRegions[surfaceIndex]};
                surfaceIndex++;
                if (region == NO_REGION) {
                    continue;
                }

                for (auto [xOffset, yOffset] : OFFSETS) {
                    int neighborX{x + xOffset};
                    int neighborY{y + yOffset};
                    if ((neighborX < gridExtent.x)
                        || (neighborX > gridExtent.xMax())
                        || (neighborY < gridExtent.y)
                        || (neighborY > gridExtent.yMax())) {
                        continue;
                    }

                    int neighborChunkIndex{
                        navGrid.getChunkIndex(neighborX, neighborY)};
                    const ChunkRegions& neighborChunk{
                        chunks[neighborChunkIndex]};
                    for (const NavSurface& neighborSurface :
                         navGrid.getSurfaces(neighborX, neighborY)) {
                        if (!canStep(surface, neighborSurface, xOffset,
                                     yOffset)) {
                            continue;
                        }

                        // Note: canStep() ensures that the neighbor is
                        //       standable, so it's in a region.
                        Uint16 neighborRegion{
                            neighborChunk
                                .surfaceRegions[navGrid.getSurfaceIndex(
                                    neighborChunkIndex, neighborSurface)]};
                        if ((neighborChunkIndex != chunkIndex)
                            || (neighborRegion != region)) {
                            linkScratch.push_back(
                                {region,
                                 {static_cast<Uint32>(neighborChunkIndex),
                                  neighborRegion}});
                        }
                    }
                }
            }
        }
    }

    // Sort the links by region and remove duplicates.
    std::ranges::sort(linkScratch);
    auto duplicates{std::ranges::unique(linkScratch)};
    linkScratch.erase(duplicates.begin(), duplicates.end());

    chunk.linkOffsets.assign(chunk.regionCount + 1, 0);
    chunk.links.clear();
    for (const auto& [region, link] : linkScratch) {
        chunk.linkOffsets[region + 1]++;
        chunk.links.push_back(link);
    }
    for (std::size_t i{1}; i < chunk.linkOffsets.size(); ++i) {
        chunk.linkOffsets[i] += chunk.linkOffsets[i - 1];
    }
}

void NavRegions::calcRegionBases()
{
    regionBases.resize(chunks.size());
    Uint32 regionCount{0};
    for (std::size_t i{0}; i < chunks.size(); ++i) {
        regionBases[i] = regionCount;
        regionCount += chunks[i].regionCount;
    }

    // Note: Existing marks may now be misaligned with their regions, so we
    //       reset them all.
    visitedMarks.assign(regionCount, 0);
    searchID = 0;
}

std::pair<int, int> NavRegions::getChunkOrigin(int chunkIndex) const
{
    static constexpr int CHUNK_WIDTH{
        static_cast<int>(SharedConfig::CHUNK_WIDTH)};

    const TileExtent& gridExtent{navGrid.getTileExtent()};
    int chunkXLength{gridExtent.xLength / CHUNK_WIDTH};
    return {gridExtent.x + ((chunkIndex % chunkXLength) * CHUNK_WIDTH),
            gridExtent.y + ((chunkIndex / chunkXLength) * CHUNK_WIDTH)};
}

bool NavRegions::isStandable(const NavSurface& surface) const
{
    return (surface.headroom
            >= std::min(SharedConfig::PATHFINDING_COMMON_HEIGHT_THIRDS,
                        static_cast<unsigned int>(NavSurface::MAX_HEADROOM)));
}

bool NavRegions::canStep(const NavSurface& fromSurface,
                         const NavSurface& toSurface, int xOffset,
                         int yOffset) const
{
    static constexpr int HEIGHT{
        static_cast<int>(SharedConfig::PATHFINDING_COMMON_HEIGHT_THIRDS)};
    static constexpr int CLIMB{
        static_cast<int>(SharedConfig::PATHFINDING_COMMON_CLIMB_THIRDS)};
    static constexpr int MAX_DROP{
        static_cast<int>(SharedConfig::PATHFINDING_COMMON_MAX_DROP_THIRDS)};
    static constexpr int MAX_HEADROOM{NavSurface::MAX_HEADROOM};

    // The height change must be within the agent's limits.
    int fromHeight{fromSurface.height};
    int toHeight{toSurface.height};
    if ((toHeight > (fromHeight + CLIMB))
        || (toHeight < (fromHeight - MAX_DROP))) {
        return false;
    }

    // While stepping, the agent is at the higher of the two heights, so both
    // surfaces need room for it there.
    int topHeight{std::max(fromHeight, toHeight)};
    if ((fromSurface.headroom
         < std::min((topHeight - fromHeight) + HEIGHT, MAX_HEADROOM))
        || (toSurface.headroom
            < std::min((topHeight - toHeight) + HEIGHT, MAX_HEADROOM))) {
        return false;
    }

    // The edge between the tiles must not be blocked.
    if (xOffset > 0) {
        return !(toSurface.isWestBlocked());
    }
    else if (xOffset < 0) {
        return !(fromSurface.isWestBlocked());
    }
    else if (yOffset > 0) {
        return !(toSurface.isNorthBlocked());
    }
    else {
        return !(fromSurface.isNorthBlocked());
    }
}

} // End namespace AM
