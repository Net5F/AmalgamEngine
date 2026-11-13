#include "NavSearch.h"
#include "NavGrid.h"
#include "NavRegions.h"
#include "AMAssert.h"
#include "tracy/Tracy.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdint>

namespace AM
{

NavSearch::NavSearch(const NavGrid& inNavGrid, NavRegions* inNavRegions)
: navGrid{inNavGrid}
, navRegions{inNavRegions}
, startPosition{}
, goalPosition{}
, agent{}
, maxExpandedNodes{0}
, goalHeight{0}
, gridVersion{0}
, status{Status::Idle}
, expandedNodeCount{0}
, lastRunExpansionCount{0}
, records{}
, recordSlots(static_cast<std::size_t>(1) << INITIAL_SLOT_BITS)
, recordSlotBits{INITIAL_SLOT_BITS}
, searchID{0}
, openList{}
, waypoints{}
, pathRecordIndices{}
, candidateHeights{}
{
}

NavSearch::Status NavSearch::findPath(const TilePosition& start,
                                      const TilePosition& goal,
                                      const NavAgentProfile& inAgent,
                                      std::size_t inMaxExpandedNodes)
{
    begin(start, goal, inAgent, inMaxExpandedNodes);
    return run(SIZE_MAX);
}

void NavSearch::begin(const TilePosition& start, const TilePosition& goal,
                      const NavAgentProfile& inAgent,
                      std::size_t inMaxExpandedNodes)
{
    AM_ASSERT(
        (inAgent.footprintTiles >= 1)
            && (inAgent.footprintTiles <= NavAgentProfile::MAX_FOOTPRINT_TILES),
        "Invalid agent footprint.");

    startPosition = start;
    goalPosition = goal;
    agent = inAgent;
    maxExpandedNodes = inMaxExpandedNodes;
    gridVersion = navGrid.getVersion();
    expandedNodeCount = 0;
    records.clear();
    openList.clear();
    waypoints.clear();

    // Invalidate the previous search's record slots.
    searchID++;
    if (searchID == 0) {
        // The ID wrapped around, so old slots may match it. Reset them.
        std::ranges::fill(recordSlots, RecordSlot{});
        searchID = 1;
    }

    // If the agent doesn't fit at the start or goal, there's no need to
    // search.
    std::optional<int> startHeight{findRestingHeight(start)};
    if (!startHeight) {
        status = Status::InvalidStart;
        return;
    }
    std::optional<int> goalRestingHeight{findRestingHeight(goal)};
    if (!goalRestingHeight) {
        status = Status::Unreachable;
        return;
    }
    goalHeight = *goalRestingHeight;

    // If the regions show that the goal can't be reached, don't search.
    if (navRegions && navRegions->supportsAgent(agent)
        && !(navRegions->isReachable(start.x, start.y, *startHeight, goal.x,
                                     goal.y, goalHeight))) {
        status = Status::Unreachable;
        return;
    }

    relaxNode(start.x, start.y, *startHeight, 0, NO_PARENT, false);
    status = Status::InProgress;
}

NavSearch::Status NavSearch::run(std::size_t maxExpansions)
{
    lastRunExpansionCount = 0;
    if (status != Status::InProgress) {
        return status;
    }

    // If the grid changed, our records may be stale. Restart the search.
    if (navGrid.getVersion() != gridVersion) {
        begin(startPosition, goalPosition, agent, maxExpandedNodes);
        if (status != Status::InProgress) {
            return status;
        }
    }
    ZoneScoped;

    while (lastRunExpansionCount < maxExpansions) {
        if (openList.empty()) {
            status = Status::Unreachable;
            return status;
        }

        std::ranges::pop_heap(openList, IsLowerPriority{});
        Uint32 recordIndex{openList.back().recordIndex};
        openList.pop_back();

        // If this node was already expanded, this entry is stale (a cheaper
        // path to the node was found after it was added).
        NodeRecord& record{records[recordIndex]};
        if (record.isClosed) {
            continue;
        }

        if ((record.x == goalPosition.x) && (record.y == goalPosition.y)
            && (record.height == goalHeight)) {
            buildWaypoints(recordIndex);
            status = Status::Found;
            return status;
        }

        if (expandedNodeCount >= maxExpandedNodes) {
            status = Status::NodeLimitReached;
            return status;
        }

        record.isClosed = true;
        expandedNodeCount++;
        lastRunExpansionCount++;
        expandNode(recordIndex);
    }

    return status;
}

NavSearch::Status NavSearch::getStatus() const
{
    return status;
}

const std::vector<NavWaypoint>& NavSearch::getWaypoints() const
{
    return waypoints;
}

std::size_t NavSearch::getExpandedNodeCount() const
{
    return expandedNodeCount;
}

std::size_t NavSearch::getLastRunExpansionCount() const
{
    return lastRunExpansionCount;
}

void NavSearch::expandNode(Uint32 recordIndex)
{
    // Note: We copy these, since adding records may invalidate references.
    const int x{records[recordIndex].x};
    const int y{records[recordIndex].y};
    const int height{records[recordIndex].height};

    expandCardinal(recordIndex, x, y, height, 0, -1);
    expandCardinal(recordIndex, x, y, height, 1, 0);
    expandCardinal(recordIndex, x, y, height, 0, 1);
    expandCardinal(recordIndex, x, y, height, -1, 0);

    expandDiagonal(recordIndex, x, y, height, 1, -1);
    expandDiagonal(recordIndex, x, y, height, 1, 1);
    expandDiagonal(recordIndex, x, y, height, -1, 1);
    expandDiagonal(recordIndex, x, y, height, -1, -1);
}

void NavSearch::expandCardinal(Uint32 recordIndex, int fromX, int fromY,
                               int fromHeight, int xOffset, int yOffset)
{
    const int toX{fromX + xOffset};
    const int toY{fromY + yOffset};
    const float fromGCost{records[recordIndex].gCost};

    // Try each height that the agent could rest at after the step.
    findCandidateHeights(toX, toY, fromHeight - agent.maxDropThirds,
                         fromHeight + agent.climbThirds);
    for (int toHeight : candidateHeights) {
        if (!fitsFootprint(toX, toY, toHeight, toHeight)) {
            continue;
        }

        // While stepping, the agent is at the higher of the two heights, so
        // both footprints need room for it at that height.
        // Note: The agent already fits at each footprint's own height.
        int topHeight{std::max(fromHeight, toHeight)};
        if (((topHeight > fromHeight)
             && !fitsFootprint(fromX, fromY, fromHeight, topHeight))
            || ((topHeight > toHeight)
                && !fitsFootprint(toX, toY, toHeight, topHeight))) {
            continue;
        }

        // Note: For agents wider than 1 tile, the crossed edges are inside
        //       the destination footprint, which fitsFootprint() checked.
        if (agent.footprintTiles == 1) {
            const NavSurface* fromSurface{
                navGrid.findSurface(fromX, fromY, fromHeight)};
            const NavSurface* toSurface{
                navGrid.findSurface(toX, toY, toHeight)};
            if (isEdgeBlocked(fromX, fromY, *fromSurface, toX, toY,
                              *toSurface)) {
                continue;
            }
        }

        bool isJump{toHeight > fromHeight};
        float stepCost{1.f
                       + (isJump ? SharedConfig::PATHFINDING_JUMP_COST : 0.f)};
        relaxNode(toX, toY, toHeight, fromGCost + stepCost, recordIndex,
                  isJump);
    }
}

void NavSearch::expandDiagonal(Uint32 recordIndex, int fromX, int fromY,
                               int height, int xOffset, int yOffset)
{
    static constexpr float DIAGONAL_COST{1.41421356f};

    // Diagonal steps must be flat, and the agent must fit at all 3 anchors
    // (to avoid cutting corners).
    const int toX{fromX + xOffset};
    const int toY{fromY + yOffset};
    if (!fitsFootprint(toX, fromY, height, height)
        || !fitsFootprint(fromX, toY, height, height)
        || !fitsFootprint(toX, toY, height, height)) {
        return;
    }

    // Note: For agents wider than 1 tile, the crossed edges are inside the
    //       footprints that we just checked.
    if (agent.footprintTiles == 1) {
        const NavSurface& fromSurface{
            *navGrid.findSurface(fromX, fromY, height)};
        const NavSurface& xSideSurface{
            *navGrid.findSurface(toX, fromY, height)};
        const NavSurface& ySideSurface{
            *navGrid.findSurface(fromX, toY, height)};
        const NavSurface& toSurface{*navGrid.findSurface(toX, toY, height)};
        if (isEdgeBlocked(fromX, fromY, fromSurface, toX, fromY, xSideSurface)
            || isEdgeBlocked(fromX, fromY, fromSurface, fromX, toY,
                             ySideSurface)
            || isEdgeBlocked(toX, fromY, xSideSurface, toX, toY, toSurface)
            || isEdgeBlocked(fromX, toY, ySideSurface, toX, toY, toSurface)) {
            return;
        }
    }

    relaxNode(toX, toY, height, records[recordIndex].gCost + DIAGONAL_COST,
              recordIndex, false);
}

void NavSearch::relaxNode(int x, int y, int height, float gCost,
                          Uint32 parentIndex, bool isJump)
{
    Uint32& recordIndexRef{getRecordIndex(x, y, height)};
    if (recordIndexRef == NO_RECORD) {
        recordIndexRef = static_cast<Uint32>(records.size());
        records.emplace_back(x, y, height, gCost, parentIndex, isJump, false);
    }
    else {
        // If we already have an equal or better path to this node, skip it.
        // Note: Our heuristic is consistent, so closed nodes never improve.
        NodeRecord& record{records[recordIndexRef]};
        if (record.isClosed || (gCost >= record.gCost)) {
            return;
        }

        record.gCost = gCost;
        record.parentIndex = parentIndex;
        record.isJump = isJump;
    }

    // Add an entry to the open list. If the node already had one, it will be
    // skipped when it's popped (see run()).
    float hCost{calcHeuristic(x, y, height)};
    openList.emplace_back(gCost + hCost, hCost, recordIndexRef);
    std::ranges::push_heap(openList, IsLowerPriority{});
}

bool NavSearch::fitsFootprint(int anchorX, int anchorY, int restingHeight,
                              int liftedHeight) const
{
    bool isSupported{false};
    for (int y{anchorY}; y < (anchorY + agent.footprintTiles); ++y) {
        for (int x{anchorX}; x < (anchorX + agent.footprintTiles); ++x) {
            // Every column needs a surface under the agent.
            const NavSurface* surface{findSurfaceBelow(x, y, restingHeight)};
            if (!surface) {
                return false;
            }

            // The surface needs room for the agent above it.
            // Note: Headroom is capped, so if we need more than the cap,
            //       accept the cap.
            int requiredHeadroom{
                std::min((liftedHeight - surface->height) + agent.heightThirds,
                         static_cast<int>(NavSurface::MAX_HEADROOM))};
            if (surface->headroom < requiredHeadroom) {
                return false;
            }

            // There must be no blocked edges between the footprint's columns.
            if (((x > anchorX) && surface->isWestBlocked())
                || ((y > anchorY) && surface->isNorthBlocked())) {
                return false;
            }

            if (surface->height == restingHeight) {
                isSupported = true;
            }
        }
    }

    // The agent rests on the highest surface in its footprint, so one must
    // be at the resting height.
    return isSupported;
}

const NavSurface* NavSearch::findSurfaceBelow(int x, int y, int height) const
{
    // Note: Surfaces are sorted by increasing height.
    const NavSurface* surfaceBelow{nullptr};
    for (const NavSurface& surface : navGrid.getSurfaces(x, y)) {
        if (surface.height > height) {
            break;
        }
        surfaceBelow = &surface;
    }

    return surfaceBelow;
}

bool NavSearch::isEdgeBlocked(int fromX, int fromY,
                              const NavSurface& fromSurface, int toX, int toY,
                              const NavSurface& toSurface) const
{
    // Each surface only holds its tile's west and north edges, so we check
    // the surface that's to the east or south.
    if (toX > fromX) {
        return toSurface.isWestBlocked();
    }
    else if (toX < fromX) {
        return fromSurface.isWestBlocked();
    }
    else if (toY > fromY) {
        return toSurface.isNorthBlocked();
    }
    else {
        return fromSurface.isNorthBlocked();
    }
}

std::optional<int>
    NavSearch::findRestingHeight(const TilePosition& tilePosition)
{
    int layerMinHeight{(tilePosition.z - navGrid.getTileExtent().z) * 3};
    findCandidateHeights(tilePosition.x, tilePosition.y, layerMinHeight,
                         layerMinHeight + 2);
    for (int height : candidateHeights) {
        if (fitsFootprint(tilePosition.x, tilePosition.y, height, height)) {
            return height;
        }
    }

    return std::nullopt;
}

void NavSearch::findCandidateHeights(int anchorX, int anchorY, int minHeight,
                                     int maxHeight)
{
    // The agent rests on the highest surface in its footprint, so each
    // surface in range is a candidate.
    candidateHeights.clear();
    for (int y{anchorY}; y < (anchorY + agent.footprintTiles); ++y) {
        for (int x{anchorX}; x < (anchorX + agent.footprintTiles); ++x) {
            for (const NavSurface& surface : navGrid.getSurfaces(x, y)) {
                if (surface.height > maxHeight) {
                    break;
                }
                else if (surface.height >= minHeight) {
                    candidateHeights.push_back(surface.height);
                }
            }
        }
    }

    std::ranges::sort(candidateHeights, std::ranges::greater{});
    auto duplicates{std::ranges::unique(candidateHeights)};
    candidateHeights.erase(duplicates.begin(), duplicates.end());
}

float NavSearch::calcHeuristic(int x, int y, int height) const
{
    static constexpr float DIAGONAL_EXTRA_COST{1.41421356f - 1.f};

    // Octile distance.
    int xDistance{std::abs(goalPosition.x - x)};
    int yDistance{std::abs(goalPosition.y - y)};
    float octileDistance{
        static_cast<float>(std::max(xDistance, yDistance))
        + (DIAGONAL_EXTRA_COST
           * static_cast<float>(std::min(xDistance, yDistance)))};

    // Each step can only climb or drop so far, so the height difference
    // requires a minimum number of steps. Only cardinal steps change height,
    // and climbing steps cost extra.
    // Note: Each step changes this estimate by at most its own cost, so the
    //       heuristic stays consistent.
    // TODO: Test whether this height term is beneficial on real maps (e.g.
    //       compare expanded node counts with and without it, on
    //       multi-floor buildings). In a small single-tower test it made no
    //       difference. If it isn't beneficial, remove it.
    float heightSteps{0};
    int heightDifference{goalHeight - height};
    if ((heightDifference > 0) && (agent.climbThirds > 0)) {
        int stepCount{(heightDifference + agent.climbThirds - 1)
                      / agent.climbThirds};
        heightSteps = stepCount * (1.f + SharedConfig::PATHFINDING_JUMP_COST);
    }
    else if ((heightDifference < 0) && (agent.maxDropThirds > 0)) {
        int stepCount{(-heightDifference + agent.maxDropThirds - 1)
                      / agent.maxDropThirds};
        heightSteps = static_cast<float>(stepCount);
    }

    return std::max(octileDistance, heightSteps);
}

Uint64 NavSearch::calcKey(int x, int y, int height) const
{
    // Note: Grids are at most 2^24 tiles wide, and heights are at most 2^16
    //       thirds.
    const TileExtent& gridExtent{navGrid.getTileExtent()};
    Uint64 positiveX{static_cast<Uint64>(x - gridExtent.x)};
    Uint64 positiveY{static_cast<Uint64>(y - gridExtent.y)};
    return (positiveX << 40) | (positiveY << 16) | static_cast<Uint64>(height);
}

Uint32& NavSearch::getRecordIndex(int x, int y, int height)
{
    // Keep the table at most half full, so that probes stay short.
    if (((records.size() + 1) * 2) > recordSlots.size()) {
        growRecordSlots();
    }

    Uint64 key{calcKey(x, y, height)};
    RecordSlot& slot{findSlot(key)};
    if (slot.searchID != searchID) {
        slot.key = key;
        slot.recordIndex = NO_RECORD;
        slot.searchID = searchID;
    }

    return slot.recordIndex;
}

NavSearch::RecordSlot& NavSearch::findSlot(Uint64 key)
{
    // Fibonacci hashing, with linear probing.
    static constexpr Uint64 HASH_MULTIPLIER{11400714819323198485ULL};
    std::size_t mask{recordSlots.size() - 1};
    std::size_t slotIndex{static_cast<std::size_t>((key * HASH_MULTIPLIER)
                                                   >> (64 - recordSlotBits))};
    while (true) {
        RecordSlot& slot{recordSlots[slotIndex]};
        if ((slot.searchID != searchID) || (slot.key == key)) {
            return slot;
        }
        slotIndex = (slotIndex + 1) & mask;
    }
}

void NavSearch::growRecordSlots()
{
    // Note: This only allocates until the table fits the largest search
    //       we've done.
    recordSlotBits++;
    recordSlots.assign(static_cast<std::size_t>(1) << recordSlotBits,
                       RecordSlot{});

    // Note: The new slots have searchID 0, and searchID is never 0, so they
    //       start out empty.
    for (Uint32 i{0}; i < records.size(); ++i) {
        const NodeRecord& record{records[i]};
        Uint64 key{calcKey(record.x, record.y, record.height)};
        RecordSlot& slot{findSlot(key)};
        slot.key = key;
        slot.recordIndex = i;
        slot.searchID = searchID;
    }
}

void NavSearch::buildWaypoints(Uint32 goalRecordIndex)
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};

    // Walk back from the goal to the start.
    pathRecordIndices.clear();
    for (Uint32 recordIndex{goalRecordIndex}; recordIndex != NO_PARENT;
         recordIndex = records[recordIndex].parentIndex) {
        pathRecordIndices.push_back(recordIndex);
    }
    std::ranges::reverse(pathRecordIndices);

    // Add a waypoint for each node after the start, skipping nodes in the
    // middle of a straight, flat line.
    const int gridZ{navGrid.getTileExtent().z};
    const float halfFootprintWidth{(agent.footprintTiles * TILE_WIDTH) / 2.f};
    waypoints.clear();
    for (std::size_t i{1}; i < pathRecordIndices.size(); ++i) {
        const NodeRecord& record{records[pathRecordIndices[i]]};
        if ((i + 1) < pathRecordIndices.size()) {
            const NodeRecord& previous{records[pathRecordIndices[i - 1]]};
            const NodeRecord& next{records[pathRecordIndices[i + 1]]};
            bool isStraight{
                ((record.x - previous.x) == (next.x - record.x))
                && ((record.y - previous.y) == (next.y - record.y))};
            bool isFlat{(previous.height == record.height)
                        && (record.height == next.height)};
            if (isStraight && isFlat) {
                continue;
            }
        }

        waypoints.emplace_back(
            TilePosition{record.x, record.y,
                         gridZ + floorDivide(record.height, 3)},
            Vector3{(record.x * TILE_WIDTH) + halfFootprintWidth,
                    (record.y * TILE_WIDTH) + halfFootprintWidth,
                    navGrid.getWorldZ(record.height)},
            record.isJump);
    }
}

int NavSearch::floorDivide(int a, int b) const
{
    int quotient{a / b};
    if (((a % b) != 0) && ((a < 0) != (b < 0))) {
        quotient--;
    }
    return quotient;
}

} // End namespace AM
