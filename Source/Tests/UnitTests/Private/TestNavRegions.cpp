#include "catch2/catch_all.hpp"
#include "NavTestMap.h"
#include "NavRegions.h"
#include "NavSearch.h"
#include <optional>
#include <vector>

using namespace AM;

namespace
{
/**
 * A test map with regions.
 */
struct RegionTestMap : public NavTestMap {
    NavRegions regions{navGrid};

    void buildRegions()
    {
        build();
        regions.build();
    }

    void updateRegions()
    {
        update();
        regions.update();
    }

    /** Returns the height of the highest surface in the given column. */
    std::optional<int> topHeight(int x, int y)
    {
        std::span<const NavSurface> surfaces{navGrid.getSurfaces(x, y)};
        if (surfaces.empty()) {
            return std::nullopt;
        }
        return surfaces.back().height;
    }

    /** Returns true if the top surface in the start column can reach the top
        surface in the goal column. */
    bool isReachable(int startX, int startY, int goalX, int goalY)
    {
        return regions.isReachable(startX, startY, *topHeight(startX, startY),
                                   goalX, goalY, *topHeight(goalX, goalY));
    }
};
} // namespace

TEST_CASE_METHOD(RegionTestMap, "NavRegions walls")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A wall across the map.
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        addWall({1, y, 0}, Wall::Type::West);
    }
    buildRegions();

    // Each side can reach itself, but not the other side (including across
    // chunks).
    REQUIRE(isReachable(-10, -10, 0, 10));
    REQUIRE(isReachable(5, -10, 10, 10));
    REQUIRE(!isReachable(0, 0, 1, 0));
    REQUIRE(!isReachable(10, 10, -10, -10));

    // Open a gap in the wall.
    remWalls({1, 5, 0});
    updateRegions();
    REQUIRE(isReachable(0, 0, 1, 0));
    REQUIRE(isReachable(10, 10, -10, -10));

    // Close it again.
    addWall({1, 5, 0}, Wall::Type::West);
    updateRegions();
    REQUIRE(!isReachable(0, 0, 1, 0));
}

TEST_CASE_METHOD(RegionTestMap, "NavRegions one-way drops")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A ledge that's too high to climb, covering the east half of the map.
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        for (int x{2}; x <= extent.xMax(); ++x) {
            addTerrain({x, y, 0}, Terrain::Height::TwoThird);
        }
    }
    buildRegions();

    // The agent can drop off the ledge, but not climb back up.
    REQUIRE(isReachable(10, 0, -10, 0));
    REQUIRE(!isReachable(-10, 0, 10, 0));
}

TEST_CASE_METHOD(RegionTestMap, "NavRegions unknown reachability")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A ceiling that's too low for the common agent to stand under.
    addTileEntity({0, 0, 0}, {{0, 0, (TILE_HEIGHT / 3) + 1},
                              {TILE_WIDTH, TILE_WIDTH, TILE_HEIGHT}});
    buildRegions();
    REQUIRE(navGrid.findSurface(0, 0, 0)->headroom
            < SharedConfig::PATHFINDING_COMMON_HEIGHT_THIRDS);

    // Surfaces that the common agent can't stand on (or that don't exist)
    // aren't in a region, so reachability is unknown.
    REQUIRE(regions.isReachable(0, 0, 0, 5, 5, 0));
    REQUIRE(regions.isReachable(5, 5, 0, 5, 5, 3));
}

TEST_CASE_METHOD(RegionTestMap, "NavRegions match NavSearch")
{
    // A map with walls, ledges, stairs, a pit, and an obstacle.
    fillTerrain(0, Terrain::Height::Flat);
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        if ((y != -8) && (y != 9)) {
            addWall({-6, y, 0}, Wall::Type::West);
        }
    }
    for (int y{-4}; y <= 4; ++y) {
        for (int x{6}; x <= 12; ++x) {
            addTerrain({x, y, 0}, Terrain::Height::TwoThird);
        }
        addTerrain({5, y, 0}, Terrain::Height::OneThird);
        remTerrain({-12, y, 0});
    }
    addTerrain({6, -4, 0}, Terrain::Height::Full);
    addTileEntity({0, 0, 0},
                  {{0, 0, 0}, {TILE_WIDTH, TILE_WIDTH, TILE_HEIGHT}});
    buildRegions();

    // For every pair of sample columns, regions should agree with a search.
    NavSearch search{navGrid};
    NavAgentProfile agent{};
    REQUIRE(regions.supportsAgent(agent));
    int pairCount{0};
    for (int startY{extent.y}; startY <= extent.yMax(); startY += 3) {
        for (int startX{extent.x}; startX <= extent.xMax(); startX += 3) {
            for (int goalY{extent.y}; goalY <= extent.yMax(); goalY += 5) {
                for (int goalX{extent.x}; goalX <= extent.xMax(); goalX += 5) {
                    std::optional<int> startHeight{topHeight(startX, startY)};
                    std::optional<int> goalHeight{topHeight(goalX, goalY)};
                    if (!startHeight || !goalHeight) {
                        continue;
                    }

                    TilePosition start{startX, startY, *startHeight / 3};
                    TilePosition goal{goalX, goalY, *goalHeight / 3};
                    NavSearch::Status status{
                        search.findPath(start, goal, agent, 100'000)};
                    if (status == NavSearch::Status::InvalidStart) {
                        continue;
                    }

                    INFO("(" << startX << ", " << startY << ") -> (" << goalX
                             << ", " << goalY << ")");
                    bool isFound{status == NavSearch::Status::Found};
                    REQUIRE(isFound
                            == regions.isReachable(startX, startY, *startHeight,
                                                   goalX, goalY, *goalHeight));
                    pairCount++;
                }
            }
        }
    }
    REQUIRE(pairCount > 1000);
}

TEST_CASE_METHOD(RegionTestMap, "NavRegions updates match full builds")
{
    fillTerrain(0, Terrain::Height::Flat);
    buildRegions();

    // Make a series of changes, including on chunk boundaries.
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        addWall({0, y, 0}, Wall::Type::West);
    }
    for (int x{extent.x}; x <= extent.xMax(); ++x) {
        addTerrain({x, 0, 0}, Terrain::Height::TwoThird);
    }
    remWalls({0, 8, 0});
    updateRegions();

    NavRegions fullRegions{navGrid};
    fullRegions.build();
    for (int startY{extent.y}; startY <= extent.yMax(); startY += 4) {
        for (int startX{extent.x}; startX <= extent.xMax(); startX += 4) {
            for (int goalY{extent.y}; goalY <= extent.yMax(); goalY += 4) {
                for (int goalX{extent.x}; goalX <= extent.xMax(); goalX += 4) {
                    int startHeight{*topHeight(startX, startY)};
                    int goalHeight{*topHeight(goalX, goalY)};
                    INFO("(" << startX << ", " << startY << ") -> (" << goalX
                             << ", " << goalY << ")");
                    REQUIRE(regions.isReachable(startX, startY, startHeight,
                                                goalX, goalY, goalHeight)
                            == fullRegions.isReachable(startX, startY,
                                                       startHeight, goalX,
                                                       goalY, goalHeight));
                }
            }
        }
    }
}

TEST_CASE_METHOD(RegionTestMap, "NavRegions skip searches")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A box of walls around (5, 5).
    addWall({5, 5, 0}, Wall::Type::West);
    addWall({5, 5, 0}, Wall::Type::North);
    addWall({6, 5, 0}, Wall::Type::West);
    addWall({5, 6, 0}, Wall::Type::North);
    buildRegions();

    // With regions, the search fails without expanding any nodes.
    NavSearch searchWithRegions{navGrid, &regions};
    REQUIRE(searchWithRegions.findPath({0, 0, 0}, {5, 5, 0})
            == NavSearch::Status::Unreachable);
    REQUIRE(searchWithRegions.getExpandedNodeCount() == 0);

    // Without them, it searches the whole map first.
    NavSearch searchWithoutRegions{navGrid};
    REQUIRE(searchWithoutRegions.findPath({0, 0, 0}, {5, 5, 0})
            == NavSearch::Status::Unreachable);
    REQUIRE(searchWithoutRegions.getExpandedNodeCount() > 1000);

    // Agents that can go places that the common agent can't aren't
    // supported.
    NavAgentProfile climber{};
    climber.climbThirds = SharedConfig::PATHFINDING_COMMON_CLIMB_THIRDS + 1;
    REQUIRE(!(regions.supportsAgent(climber)));
    NavAgentProfile largeAgent{};
    largeAgent.footprintTiles = 2;
    REQUIRE(!(regions.supportsAgent(largeAgent)));
}
