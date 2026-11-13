#include "catch2/catch_all.hpp"
#include "NavTestMap.h"
#include "NavSearch.h"
#include <algorithm>
#include <array>
#include <utility>
#include <vector>

using namespace AM;

namespace
{
/**
 * A test map with a search.
 */
struct SearchTestMap : public NavTestMap {
    NavSearch search{navGrid};

    /** Fills the given tile with a full-tile obstacle. */
    void addObstacle(const TilePosition& tilePosition)
    {
        addTileEntity(tilePosition,
                      {{0, 0, 0}, {TILE_WIDTH, TILE_WIDTH, TILE_HEIGHT}});
    }

    /** Fills the given column (all Y values) with terrain of the given
        height. */
    void fillColumn(int x, int z, Terrain::Height height)
    {
        for (int y{extent.y}; y <= extent.yMax(); ++y) {
            addTerrain({x, y, z}, height);
        }
    }

    NavSearch::Status findPath(const TilePosition& start,
                               const TilePosition& goal,
                               const NavAgentProfile& agent = {})
    {
        return search.findPath(start, goal, agent);
    }

    const std::vector<NavWaypoint>& waypoints()
    {
        return search.getWaypoints();
    }
};
} // namespace

TEST_CASE_METHOD(SearchTestMap, "NavSearch straight lines")
{
    fillTerrain(0, Terrain::Height::Flat);
    build();

    SECTION("Cardinal")
    {
        REQUIRE(findPath({0, 0, 0}, {5, 0, 0}) == NavSearch::Status::Found);

        // Collinear waypoints are skipped, leaving only the goal.
        REQUIRE(waypoints().size() == 1);
        REQUIRE(waypoints()[0].tilePosition == TilePosition{5, 0, 0});
        REQUIRE(waypoints()[0].position
                == Vector3{(5 * TILE_WIDTH) + (TILE_WIDTH / 2),
                           (TILE_WIDTH / 2), 0});
        REQUIRE(!(waypoints()[0].isJump));
    }

    SECTION("Diagonal")
    {
        REQUIRE(findPath({0, 0, 0}, {-4, 4, 0}) == NavSearch::Status::Found);
        REQUIRE(waypoints().size() == 1);
        REQUIRE(waypoints()[0].tilePosition == TilePosition{-4, 4, 0});
    }

    SECTION("Start is the goal")
    {
        REQUIRE(findPath({0, 0, 0}, {0, 0, 0}) == NavSearch::Status::Found);
        REQUIRE(waypoints().empty());
    }
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch invalid endpoints")
{
    fillTerrain(0, Terrain::Height::Flat);
    addObstacle({3, 0, 0});
    build();

    // Starting in the air or inside an obstacle.
    REQUIRE(findPath({0, 0, 1}, {5, 0, 0}) == NavSearch::Status::InvalidStart);
    REQUIRE(findPath({3, 0, 0}, {5, 0, 0}) == NavSearch::Status::InvalidStart);

    // Going to the air or inside an obstacle.
    REQUIRE(findPath({0, 0, 0}, {5, 0, 1}) == NavSearch::Status::Unreachable);
    REQUIRE(findPath({0, 0, 0}, {3, 0, 0}) == NavSearch::Status::Unreachable);

    // Out of bounds.
    REQUIRE(findPath({0, 0, 0}, {100, 0, 0}) == NavSearch::Status::Unreachable);
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch obstacles and corner cutting")
{
    fillTerrain(0, Terrain::Height::Flat);
    addObstacle({1, 0, 0});
    build();

    // The diagonal step would cut the obstacle's corner, so the path must go
    // around it.
    REQUIRE(findPath({0, 0, 0}, {1, 1, 0}) == NavSearch::Status::Found);
    REQUIRE(waypoints().size() == 2);
    REQUIRE(waypoints()[0].tilePosition == TilePosition{0, 1, 0});
    REQUIRE(waypoints()[1].tilePosition == TilePosition{1, 1, 0});
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch walls")
{
    fillTerrain(0, Terrain::Height::Flat);

    SECTION("Blocked edge")
    {
        addWall({1, 0, 0}, Wall::Type::West);
        build();

        // The direct step is blocked, so the path must go around the wall.
        REQUIRE(findPath({0, 0, 0}, {1, 0, 0}) == NavSearch::Status::Found);
        REQUIRE(waypoints().size() > 1);

        // Diagonals can't cut past the end of the wall.
        REQUIRE(findPath({0, 0, 0}, {1, 1, 0}) == NavSearch::Status::Found);
        REQUIRE(waypoints().size() > 1);
    }

    SECTION("Wall with a gap")
    {
        // A wall across the map, with a gap at y == 0.
        for (int y{extent.y}; y <= extent.yMax(); ++y) {
            if (y != 0) {
                addWall({1, y, 0}, Wall::Type::West);
            }
        }
        build();

        REQUIRE(findPath({-2, -5, 0}, {3, -5, 0}) == NavSearch::Status::Found);

        // Close the gap.
        addWall({1, 0, 0}, Wall::Type::West);
        update();
        REQUIRE(findPath({-2, -5, 0}, {3, -5, 0})
                == NavSearch::Status::Unreachable);
    }
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch climbing")
{
    fillTerrain(0, Terrain::Height::Flat);

    SECTION("Jumping onto a ledge")
    {
        fillColumn(1, 0, Terrain::Height::OneThird);
        build();

        REQUIRE(findPath({0, 0, 0}, {1, 0, 0}) == NavSearch::Status::Found);
        REQUIRE(waypoints().size() == 1);
        REQUIRE(waypoints()[0].isJump);
        REQUIRE(waypoints()[0].position.z == (TILE_HEIGHT / 3));

        // Walking off the ledge doesn't need a jump.
        REQUIRE(findPath({1, 0, 0}, {0, 0, 0}) == NavSearch::Status::Found);
        REQUIRE(waypoints().size() == 1);
        REQUIRE(!(waypoints()[0].isJump));
    }

    SECTION("Stairs")
    {
        fillColumn(1, 0, Terrain::Height::OneThird);
        fillColumn(2, 0, Terrain::Height::TwoThird);
        fillColumn(3, 0, Terrain::Height::Full);
        build();

        // Each step is a jump. The top of the full terrain is in the tile
        // above it.
        REQUIRE(findPath({0, 0, 0}, {3, 0, 1}) == NavSearch::Status::Found);
        REQUIRE(waypoints().size() == 3);
        for (const NavWaypoint& waypoint : waypoints()) {
            REQUIRE(waypoint.isJump);
        }
        REQUIRE(waypoints()[2].tilePosition == TilePosition{3, 0, 1});
        REQUIRE(waypoints()[2].position.z == TILE_HEIGHT);
    }

    SECTION("Too high to climb")
    {
        fillColumn(1, 0, Terrain::Height::TwoThird);
        build();

        REQUIRE(findPath({0, 0, 0}, {1, 0, 0})
                == NavSearch::Status::Unreachable);

        // Dropping down is fine.
        REQUIRE(findPath({1, 0, 0}, {0, 0, 0}) == NavSearch::Status::Found);
    }

    SECTION("Jumping needs headroom")
    {
        // A ledge, with a low ceiling in front of it. The ceiling leaves
        // enough room to stand, but not to jump.
        fillColumn(1, 0, Terrain::Height::OneThird);
        addEntity(
            {{0, extent.y * TILE_WIDTH, (TILE_HEIGHT * 2 / 3)},
             {TILE_WIDTH, (extent.yMax() + 1) * TILE_WIDTH, TILE_HEIGHT}});
        build();

        REQUIRE(navGrid.findSurface(0, 0, 0)->headroom == 2);
        REQUIRE(findPath({0, 0, 0}, {1, 0, 0})
                == NavSearch::Status::Unreachable);
    }
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch dropping")
{
    fillTerrain(0, Terrain::Height::Flat);
    addTerrain({0, 0, 0}, Terrain::Height::Full);
    build();

    // A drop of 3 thirds is within the limit, but climbing back isn't.
    REQUIRE(findPath({0, 0, 1}, {3, 0, 0}) == NavSearch::Status::Found);
    REQUIRE(findPath({3, 0, 0}, {0, 0, 1}) == NavSearch::Status::Unreachable);

    // With a lower drop limit, the agent can't get down.
    NavAgentProfile timidAgent{};
    timidAgent.maxDropThirds = 2;
    REQUIRE(findPath({0, 0, 1}, {3, 0, 0}, timidAgent)
            == NavSearch::Status::Unreachable);
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch large agents")
{
    fillTerrain(0, Terrain::Height::Flat);

    NavAgentProfile largeAgent{};
    largeAgent.footprintTiles = 2;

    // A line of obstacles across the map, with a gap at y == 0.
    int gapWidth{GENERATE(1, 2)};
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        if ((y < 0) || (y >= gapWidth)) {
            addObstacle({0, y, 0});
        }
    }
    build();

    // Small agents always fit through the gap.
    REQUIRE(findPath({-3, 0, 0}, {3, 0, 0}) == NavSearch::Status::Found);

    if (gapWidth == 1) {
        REQUIRE(findPath({-3, 0, 0}, {3, 0, 0}, largeAgent)
                == NavSearch::Status::Unreachable);
    }
    else {
        REQUIRE(findPath({-3, 0, 0}, {3, 0, 0}, largeAgent)
                == NavSearch::Status::Found);

        // Large agents' waypoints are centered on their footprint.
        const NavWaypoint& goalWaypoint{waypoints().back()};
        REQUIRE(goalWaypoint.tilePosition == TilePosition{3, 0, 0});
        REQUIRE(goalWaypoint.position
                == Vector3{(3 * TILE_WIDTH) + TILE_WIDTH, TILE_WIDTH, 0});
    }
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch large agents need headroom")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A low ceiling over one tile.
    addTileEntity({1, 1, 0}, {{0, 0, (TILE_HEIGHT / 3) + 1},
                              {TILE_WIDTH, TILE_WIDTH, TILE_HEIGHT}});
    build();

    NavAgentProfile largeAgent{};
    largeAgent.footprintTiles = 2;

    // The ceiling's tile is standable, but the agent doesn't fit under it.
    REQUIRE(navGrid.findSurface(1, 1, 0) != nullptr);
    REQUIRE(findPath({0, 0, 0}, {5, 5, 0}, largeAgent)
            == NavSearch::Status::InvalidStart);
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch across multiple runs")
{
    fillTerrain(0, Terrain::Height::Flat);
    for (int y{extent.y}; y <= 10; ++y) {
        addObstacle({0, y, 0});
    }
    build();

    REQUIRE(findPath({-5, 0, 0}, {5, 0, 0}) == NavSearch::Status::Found);
    std::vector<NavWaypoint> fullWaypoints{waypoints()};
    std::size_t fullExpandedCount{search.getExpandedNodeCount()};

    // Running the same search a few nodes at a time gives the same result.
    search.begin({-5, 0, 0}, {5, 0, 0});
    int runCount{0};
    while (search.run(5) == NavSearch::Status::InProgress) {
        runCount++;
    }
    REQUIRE(runCount > 1);
    REQUIRE(search.getStatus() == NavSearch::Status::Found);
    REQUIRE(search.getExpandedNodeCount() == fullExpandedCount);
    REQUIRE(waypoints().size() == fullWaypoints.size());
    for (std::size_t i{0}; i < fullWaypoints.size(); ++i) {
        REQUIRE(waypoints()[i].tilePosition == fullWaypoints[i].tilePosition);
    }

    // The node limit stops long searches.
    REQUIRE(search.findPath({-5, 0, 0}, {5, 0, 0}, {}, 10)
            == NavSearch::Status::NodeLimitReached);
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch reuse")
{
    fillTerrain(0, Terrain::Height::Flat);
    for (int y{extent.y}; y <= 10; ++y) {
        addObstacle({0, y, 0});
    }
    build();

    // Each search should give the same result as a fresh search object, even
    // after previous searches covered different areas.
    const std::vector<std::pair<TilePosition, TilePosition>> endpoints{
        {{-5, 0, 0}, {5, 0, 0}},
        {{-15, -15, 0}, {15, 15, 0}},
        {{10, -10, 0}, {-10, -12, 0}},
        {{-5, 0, 0}, {5, 0, 0}}};
    for (const auto& [start, goal] : endpoints) {
        NavSearch freshSearch{navGrid};
        REQUIRE(freshSearch.findPath(start, goal) == NavSearch::Status::Found);
        REQUIRE(findPath(start, goal) == NavSearch::Status::Found);

        const std::vector<NavWaypoint>& freshWaypoints{
            freshSearch.getWaypoints()};
        REQUIRE(waypoints().size() == freshWaypoints.size());
        for (std::size_t i{0}; i < freshWaypoints.size(); ++i) {
            REQUIRE(waypoints()[i].tilePosition
                    == freshWaypoints[i].tilePosition);
        }
    }

    // Searches still work after the grid is resized.
    TileExtent smallerExtent{-16, -16, 0, 16, 32, 4};
    navGrid.setGridSize(smallerExtent);
    navGrid.build();
    REQUIRE(findPath({-10, -10, 0}, {-10, 10, 0}) == NavSearch::Status::Found);
    REQUIRE(findPath({-10, -10, 0}, {5, 0, 0})
            == NavSearch::Status::Unreachable);
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch large agents changing height")
{
    fillTerrain(0, Terrain::Height::Flat);

    NavAgentProfile largeAgent{};
    largeAgent.footprintTiles = 2;

    // Raise everything east of x == 1 into a step.
    auto raiseStep = [&](Terrain::Height height) {
        for (int x{2}; x <= extent.xMax(); ++x) {
            fillColumn(x, 0, height);
        }
    };

    SECTION("Climbing a step")
    {
        raiseStep(Terrain::Height::OneThird);
        build();

        REQUIRE(findPath({-3, 0, 0}, {5, 0, 0}, largeAgent)
                == NavSearch::Status::Found);
        bool hasJump{
            std::ranges::any_of(waypoints(), [](const NavWaypoint& waypoint) {
                return waypoint.isJump;
            })};
        REQUIRE(hasJump);
        REQUIRE(waypoints().back().position.z == (TILE_HEIGHT / 3));

        // Dropping back down works too.
        REQUIRE(findPath({5, 0, 0}, {-3, 0, 0}, largeAgent)
                == NavSearch::Status::Found);
        REQUIRE(waypoints().back().position.z == 0);
    }

    SECTION("Step too high to climb")
    {
        raiseStep(Terrain::Height::TwoThird);
        build();

        REQUIRE(findPath({-3, 0, 0}, {5, 0, 0}, largeAgent)
                == NavSearch::Status::Unreachable);

        // A better climber can make it.
        largeAgent.climbThirds = 2;
        REQUIRE(findPath({-3, 0, 0}, {5, 0, 0}, largeAgent)
                == NavSearch::Status::Found);
    }

    SECTION("Hanging over a low ceiling")
    {
        raiseStep(Terrain::Height::OneThird);

        // A ceiling over the lower tiles next to the step, leaving exactly
        // enough headroom for the agent to stand on them, but not to rest
        // above them on the step.
        for (int y{extent.y}; y <= extent.yMax(); ++y) {
            addTileEntity({1, y, 0}, {{0, 0, (TILE_HEIGHT / 3) * 2},
                                      {TILE_WIDTH, TILE_WIDTH, TILE_HEIGHT}});
        }
        build();

        REQUIRE(navGrid.findSurface(1, 0, 0)->headroom
                == largeAgent.heightThirds);
        REQUIRE(findPath({-3, 0, 0}, {5, 0, 0}, largeAgent)
                == NavSearch::Status::Unreachable);
    }

    SECTION("Climbing between tile layers")
    {
        // Stairs that climb onto full-height terrain, whose surface is in
        // the next tile layer.
        fillColumn(1, 0, Terrain::Height::OneThird);
        fillColumn(2, 0, Terrain::Height::TwoThird);
        for (int x{3}; x <= extent.xMax(); ++x) {
            fillColumn(x, 0, Terrain::Height::Full);
        }
        build();

        REQUIRE(findPath({-3, 0, 0}, {5, 0, 1}, largeAgent)
                == NavSearch::Status::Found);
        REQUIRE(waypoints().back().tilePosition == TilePosition{5, 0, 1});
        REQUIRE(waypoints().back().position.z == TILE_HEIGHT);

        // And back down.
        REQUIRE(findPath({5, 0, 1}, {-3, 0, 0}, largeAgent)
                == NavSearch::Status::Found);
    }
}

TEST_CASE_METHOD(SearchTestMap, "NavSearch climbing a tower")
{
    fillTerrain(0, Terrain::Height::Flat);

    // Sets the given column to the given height (in thirds, up to 6).
    auto setHeight = [&](int x, int y, int height) {
        static constexpr std::array<Terrain::Height, 4> HEIGHTS{
            Terrain::Height::Flat, Terrain::Height::OneThird,
            Terrain::Height::TwoThird, Terrain::Height::Full};
        addTerrain({x, y, 0}, HEIGHTS[std::min(height, 3)]);
        if (height > 3) {
            addTerrain({x, y, 1}, HEIGHTS[height - 3]);
        }
    };

    // A 2x2 tower, 2 tiles tall.
    for (int y{0}; y <= 1; ++y) {
        for (int x{0}; x <= 1; ++x) {
            setHeight(x, y, 6);
        }
    }

    // A bridge from the tower to the top of a staircase, which climbs
    // westward from x == 15.
    for (int x{2}; x <= 9; ++x) {
        setHeight(x, 0, 6);
    }
    for (int x{10}; x <= 15; ++x) {
        setHeight(x, 0, 6 - (x - 10));
    }
    build();

    // Walk from the base of the tower, around to the stairs, and back across
    // the bridge to the top.
    REQUIRE(findPath({0, 5, 0}, {0, 0, 2}) == NavSearch::Status::Found);
    REQUIRE(waypoints().back().tilePosition == TilePosition{0, 0, 2});
    REQUIRE(waypoints().back().position.z == (TILE_HEIGHT * 2));
    std::size_t jumpCount{static_cast<std::size_t>(
        std::ranges::count_if(waypoints(), [](const NavWaypoint& waypoint) {
            return waypoint.isJump;
        }))};
    REQUIRE(jumpCount == 6);
}
