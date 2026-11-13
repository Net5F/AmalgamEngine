#include "catch2/catch_all.hpp"
#include "NavTestMap.h"

using namespace AM;

TEST_CASE("NavSurface flags")
{
    NavSurface surface{};
    REQUIRE(!(surface.isWestBlocked()));
    REQUIRE(!(surface.isNorthBlocked()));

    surface.setWestBlocked(true);
    REQUIRE(surface.isWestBlocked());
    REQUIRE(!(surface.isNorthBlocked()));

    // Changing one flag shouldn't affect the other.
    surface.setNorthBlocked(true);
    surface.setWestBlocked(false);
    REQUIRE(!(surface.isWestBlocked()));
    REQUIRE(surface.isNorthBlocked());
}

TEST_CASE_METHOD(NavTestMap, "NavGrid flat ground")
{
    fillTerrain(0, Terrain::Height::Flat);
    build();

    // Each column has a single surface.
    std::span<const NavSurface> surfaces{navGrid.getSurfaces(0, 0)};
    REQUIRE(surfaces.size() == 1);
    REQUIRE(surfaces[0].height == 0);
    REQUIRE(surfaces[0].headroom == openHeadroom(0));
    REQUIRE(!(surfaces[0].isWestBlocked()));
    REQUIRE(!(surfaces[0].isNorthBlocked()));
    REQUIRE(navGrid.findSurface(0, 0, 0) == &(surfaces[0]));
    REQUIRE(navGrid.findSurface(0, 0, 1) == nullptr);
    REQUIRE(navGrid.getWorldZ(3) == TILE_HEIGHT);

    // Out of bounds columns are empty.
    REQUIRE(navGrid.getSurfaces(100, 0).empty());
}

TEST_CASE_METHOD(NavTestMap, "NavGrid terrain heights")
{
    fillTerrain(0, Terrain::Height::Flat);
    addTerrain({0, 0, 0}, Terrain::Height::OneThird);
    addTerrain({1, 0, 0}, Terrain::Height::TwoThird);
    addTerrain({2, 0, 0}, Terrain::Height::Full);
    build();

    REQUIRE(tileSurface(0, 0, 0).getSurfaceHeight() == 1);
    REQUIRE(tileSurface(1, 0, 0).getSurfaceHeight() == 2);

    // Full terrain's surface is at the bottom of the tile above it.
    REQUIRE(!(tileSurface(2, 0, 0).isStandable()));
    REQUIRE(tileSurface(2, 0, 1).isStandable());
    REQUIRE(tileSurface(2, 0, 1).getSurfaceHeight() == 0);
    REQUIRE(navGrid.findSurface(2, 0, 3) != nullptr);
    REQUIRE(tileSurface(2, 0, 1).getHeadroom() == openHeadroom(3));
}

TEST_CASE_METHOD(NavTestMap, "NavGrid headroom")
{
    fillTerrain(0, Terrain::Height::Flat);

    // Terrain in the tile above.
    addTerrain({0, 0, 1}, Terrain::Height::Full);

    // Floating terrain in the tile above, from 1/3 to 2/3.
    addTerrain({1, 0, 1}, Terrain::Height::OneThird, Terrain::Height::OneThird);

    // An entity 20 units above the ground (less than a third).
    addTileEntity({2, 0, 0}, {{10, 10, 20}, {20, 20, 40}});

    // An entity 30 units above the ground (more than a third).
    addTileEntity({3, 0, 0}, {{10, 10, 30}, {20, 20, 50}});

    // A moving entity (doesn't block collision).
    addTileEntity({4, 0, 0}, {{10, 10, 0}, {20, 20, 20}},
                  CollisionLayerType::NonPlayerEntity);
    build();

    REQUIRE(tileSurface(0, 0, 0).getHeadroom() == 3);

    // The floating terrain limits the headroom below it, and provides a
    // surface on top of it.
    REQUIRE(tileSurface(1, 0, 0).getSurfaceHeight() == 0);
    REQUIRE(tileSurface(1, 0, 0).getHeadroom() == 4);
    REQUIRE(tileSurface(1, 0, 1).isStandable());
    REQUIRE(tileSurface(1, 0, 1).getSurfaceHeight() == 2);

    REQUIRE(!(tileSurface(2, 0, 0).isStandable()));
    REQUIRE(tileSurface(3, 0, 0).getHeadroom() == 1);
    REQUIRE(tileSurface(4, 0, 0).getHeadroom() == openHeadroom(0));
}

TEST_CASE_METHOD(NavTestMap, "NavGrid multiple surfaces in a tile")
{
    // Full terrain, with floating terrain above it in the next tile (from 1/3
    // to 2/3). Both surfaces are in the upper tile.
    addTerrain({0, 0, 0}, Terrain::Height::Full);
    addTerrain({0, 0, 1}, Terrain::Height::OneThird, Terrain::Height::OneThird);

    // A stack of full terrain, leaving no room between the blocks.
    addTerrain({1, 0, 0}, Terrain::Height::Full);
    addTerrain({1, 0, 1}, Terrain::Height::Full);
    build();

    std::span<const NavSurface> surfaces{navGrid.getSurfaces(0, 0)};
    REQUIRE(surfaces.size() == 2);
    REQUIRE(surfaces[0].height == 3);
    REQUIRE(surfaces[0].headroom == 1);
    REQUIRE(surfaces[1].height == 5);
    REQUIRE(surfaces[1].headroom == openHeadroom(5));

    // Only the top of the stack is a surface.
    std::span<const NavSurface> stackSurfaces{navGrid.getSurfaces(1, 0)};
    REQUIRE(stackSurfaces.size() == 1);
    REQUIRE(stackSurfaces[0].height == 6);

    // Empty columns have no surfaces.
    REQUIRE(navGrid.getSurfaces(2, 0).empty());
}

TEST_CASE_METHOD(NavTestMap, "NavGrid walls")
{
    fillTerrain(0, Terrain::Height::Flat);
    addWall({0, 0, 0}, Wall::Type::West);
    addWall({0, 2, 0}, Wall::Type::North);
    addWall({0, 4, 0}, Wall::Type::NorthWestGapFill);
    addWall({0, 6, 0}, Wall::Type::NorthEastGapFill);

    // Walls on raised terrain start at the terrain's surface.
    addTerrain({0, 8, 0}, Terrain::Height::OneThird);
    addWall({0, 8, 0}, Wall::Type::West);
    build();

    // Walls block their edge, but don't affect the rest of the tile.
    auto westWallSurface{tileSurface(0, 0, 0)};
    REQUIRE(westWallSurface.isStandable());
    REQUIRE(westWallSurface.getHeadroom() == openHeadroom(0));
    REQUIRE(westWallSurface.isWestBlocked());
    REQUIRE(!(westWallSurface.isNorthBlocked()));
    REQUIRE(!(tileSurface(-1, 0, 0).isWestBlocked()));

    auto northWallSurface{tileSurface(0, 2, 0)};
    REQUIRE(northWallSurface.isNorthBlocked());
    REQUIRE(!(northWallSurface.isWestBlocked()));

    // A corner column only covers a small part of each edge.
    auto northWestGapFillSurface{tileSurface(0, 4, 0)};
    REQUIRE(northWestGapFillSurface.isStandable());
    REQUIRE(!(northWestGapFillSurface.isWestBlocked()));
    REQUIRE(!(northWestGapFillSurface.isNorthBlocked()));

    // A 3/4 north wall covers most of the north edge.
    auto northEastGapFillSurface{tileSurface(0, 6, 0)};
    REQUIRE(northEastGapFillSurface.isNorthBlocked());
    REQUIRE(!(northEastGapFillSurface.isWestBlocked()));

    REQUIRE(tileSurface(0, 8, 0).getSurfaceHeight() == 1);
    REQUIRE(tileSurface(0, 8, 0).isWestBlocked());
}

TEST_CASE_METHOD(NavTestMap, "NavGrid obstacles")
{
    fillTerrain(0, Terrain::Height::Flat);

    // An obstacle in the middle of a tile.
    addTileEntity({0, 0, 0}, {{10, 10, 0}, {20, 20, 30}});

    // A thin obstacle along a tile's west edge acts like a wall.
    addTileEntity({2, 0, 0}, {{0, 0, 0}, {4, 32, 30}});

    // An object tile layer in the middle of a tile.
    addTileEntity({4, 0, 0}, {{10, 10, 0}, {20, 20, 30}},
                  CollisionLayerType::Object);
    build();

    REQUIRE(!(tileSurface(0, 0, 0).isStandable()));

    REQUIRE(tileSurface(2, 0, 0).isStandable());
    REQUIRE(tileSurface(2, 0, 0).isWestBlocked());

    REQUIRE(!(tileSurface(4, 0, 0).isStandable()));

    // Neighbors are unaffected.
    REQUIRE(tileSurface(1, 0, 0).isStandable());
    REQUIRE(!(tileSurface(1, 0, 0).isWestBlocked()));
}

TEST_CASE_METHOD(NavTestMap, "NavGrid updates tile changes")
{
    fillTerrain(0, Terrain::Height::Flat);
    build();
    Uint32 version{navGrid.getVersion()};

    // Updating with no changes does nothing.
    update();
    REQUIRE(navGrid.getVersion() == version);

    // Add a wall on a chunk boundary.
    addWall({0, 0, 0}, Wall::Type::West);
    update();
    REQUIRE(navGrid.getVersion() != version);
    REQUIRE(tileSurface(0, 0, 0).isWestBlocked());
    requireMatchesFullBuild();

    // Raise some terrain and add a ceiling over it.
    addTerrain({5, 5, 0}, Terrain::Height::OneThird);
    addTerrain({5, 5, 1}, Terrain::Height::Full);
    update();
    REQUIRE(tileSurface(5, 5, 0).getSurfaceHeight() == 1);
    REQUIRE(tileSurface(5, 5, 0).getHeadroom() == 2);
    REQUIRE(navGrid.findSurface(5, 5, 6) != nullptr);
    requireMatchesFullBuild();

    // Remove everything that was added.
    remWalls({0, 0, 0});
    addTerrain({5, 5, 0}, Terrain::Height::Flat);
    remTerrain({5, 5, 1});
    update();
    REQUIRE(!(tileSurface(0, 0, 0).isWestBlocked()));
    REQUIRE(navGrid.getSurfaces(5, 5).size() == 1);
    REQUIRE(tileSurface(5, 5, 0).getSurfaceHeight() == 0);
    requireMatchesFullBuild();

    // Remove the ground entirely.
    remTerrain({-3, -3, 0});
    update();
    REQUIRE(navGrid.getSurfaces(-3, -3).empty());
    requireMatchesFullBuild();
}

TEST_CASE_METHOD(NavTestMap, "NavGrid updates entity changes")
{
    fillTerrain(0, Terrain::Height::Flat);
    build();

    // Add an obstacle that spans multiple chunks.
    entt::entity bigEntity{
        addEntity({{-2 * TILE_WIDTH, -2 * TILE_WIDTH, 0},
                   {2 * TILE_WIDTH, 2 * TILE_WIDTH, TILE_HEIGHT}})};
    update();
    REQUIRE(!(tileSurface(-1, -1, 0).isStandable()));
    REQUIRE(!(tileSurface(1, 1, 0).isStandable()));
    REQUIRE(tileSurface(2, 2, 0).isStandable());
    requireMatchesFullBuild();

    // Move an obstacle from one tile to another.
    entt::entity smallEntity{
        addTileEntity({8, 8, 0}, {{10, 10, 0}, {20, 20, 30}})};
    update();
    REQUIRE(!(tileSurface(8, 8, 0).isStandable()));
    moveTileEntity(smallEntity, {-8, 8, 0}, {{10, 10, 0}, {20, 20, 30}});
    update();
    REQUIRE(tileSurface(8, 8, 0).isStandable());
    REQUIRE(!(tileSurface(-8, 8, 0).isStandable()));
    requireMatchesFullBuild();

    // Remove the big obstacle.
    collisionLocator.removeEntity(bigEntity);
    update();
    REQUIRE(tileSurface(-1, -1, 0).isStandable());
    REQUIRE(tileSurface(1, 1, 0).isStandable());
    requireMatchesFullBuild();

    // Moving entities aren't tracked.
    addTileEntity({4, 4, 0}, {{10, 10, 0}, {20, 20, 30}},
                  CollisionLayerType::NonPlayerEntity);
    REQUIRE(collisionLocator.getTrackedChanges().empty());
    update();
    REQUIRE(tileSurface(4, 4, 0).isStandable());
}

namespace
{
/** A test map that's taller than the old 85-layer limit. */
struct TallNavTestMap : public NavTestMap {
    TallNavTestMap()
    : NavTestMap(TileExtent{-16, -16, 0, 32, 32, 100})
    {
    }
};
} // namespace

TEST_CASE_METHOD(TallNavTestMap, "NavGrid tall maps")
{
    // Ground at the bottom, and a platform near the top.
    fillTerrain(0, Terrain::Height::Flat);
    addTerrain({0, 0, 98}, Terrain::Height::Full);
    build();

    std::span<const NavSurface> surfaces{navGrid.getSurfaces(0, 0)};
    REQUIRE(surfaces.size() == 2);
    REQUIRE(surfaces[0].height == 0);
    REQUIRE(surfaces[0].headroom == NavSurface::MAX_HEADROOM);
    REQUIRE(surfaces[1].height == (99 * 3));
    REQUIRE(surfaces[1].headroom == 3);
}
