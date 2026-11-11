#include "catch2/catch_all.hpp"
#include "NavGrid.h"
#include "CollisionLocator.h"
#include "GraphicSets.h"
#include "Tile.h"
#include "TilePosition.h"
#include "TileExtent.h"
#include "Terrain.h"
#include "Wall.h"
#include "SharedConfig.h"
#include "entt/entity/entity.hpp"
#include <unordered_map>
#include <vector>

using namespace AM;

namespace
{
constexpr float TILE_WIDTH{static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
constexpr float TILE_HEIGHT{
    static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

/**
 * A small test map: 2x2 chunks centered on the origin, 4 tiles tall.
 *
 * Tile layers and static entities are added directly to a collision locator,
 * then the nav grid is built from it.
 */
struct TestMap {
    TileExtent extent{-16, -16, 0, 32, 32, 4};

    // Terrain graphics don't affect collision, so one sprite is enough.
    Sprite terrainSprite{};
    TerrainGraphicSet terrainSet{
        {},
        {GraphicRef{terrainSprite}, GraphicRef{terrainSprite},
         GraphicRef{terrainSprite}, GraphicRef{terrainSprite}}};

    // These bounds match the engine's default wall graphics.
    Sprite westWallSprite{.collisionEnabled{true},
                          .modelBounds{{0, 0, 0}, {5.5f, 32, 84}}};
    Sprite northWallSprite{.collisionEnabled{true},
                           .modelBounds{{0, 0, 0}, {32, 5.5f, 84}}};
    Sprite northWestGapFillSprite{.collisionEnabled{true},
                                  .modelBounds{{0, 0, 0}, {5.5f, 5.5f, 84}}};
    Sprite northEastGapFillSprite{.collisionEnabled{true},
                                  .modelBounds{{5.5f, 0, 0}, {32, 5.5f, 84}}};
    WallGraphicSet wallSet{{},
                           {GraphicRef{westWallSprite},
                            GraphicRef{northWallSprite},
                            GraphicRef{northWestGapFillSprite},
                            GraphicRef{northEastGapFillSprite}}};

    std::unordered_map<TilePosition, Terrain::Value> terrain{};
    std::unordered_map<TilePosition, std::vector<Wall::Type>> walls{};
    Uint32 nextEntityID{0};

    CollisionLocator collisionLocator{};
    NavGrid navGrid{collisionLocator};

    TestMap() { collisionLocator.setGridSize(extent); }

    void addTerrain(const TilePosition& tilePosition, Terrain::Height height,
                    Terrain::Height startHeight = Terrain::Height::Flat)
    {
        terrain[tilePosition] = Terrain::toValue(height, startHeight);
    }

    /** Fills the given Z level with terrain of the given height. */
    void fillTerrain(int z, Terrain::Height height)
    {
        for (int y{extent.y}; y <= extent.yMax(); ++y) {
            for (int x{extent.x}; x <= extent.xMax(); ++x) {
                addTerrain({x, y, z}, height);
            }
        }
    }

    void addWall(const TilePosition& tilePosition, Wall::Type wallType)
    {
        walls[tilePosition].push_back(wallType);
    }

    /** Adds an entity with the given world-space collision. */
    void addEntity(const BoundingBox& worldBounds,
                   CollisionLayerBitSet collisionLayers
                   = CollisionLayerType::BlockCollision)
    {
        collisionLocator.updateEntity(static_cast<entt::entity>(nextEntityID++),
                                      worldBounds, collisionLayers);
    }

    /** Adds an entity within the given tile, offset by the given bounds. */
    void addTileEntity(const TilePosition& tilePosition,
                       const BoundingBox& tileBounds,
                       CollisionLayerBitSet collisionLayers
                       = CollisionLayerType::BlockCollision)
    {
        addEntity(tileBounds.translateBy(tilePosition.getOriginPoint()),
                  collisionLayers);
    }

    /** Adds the tiles to the collision locator and builds the nav grid. */
    void build()
    {
        std::unordered_map<TilePosition, Tile> tiles{};
        for (const auto& [tilePosition, terrainValue] : terrain) {
            tiles[tilePosition].addLayer({}, TileLayer::Type::Terrain,
                                         terrainSet, terrainValue);
        }
        for (const auto& [tilePosition, wallTypes] : walls) {
            for (Wall::Type wallType : wallTypes) {
                tiles[tilePosition].addLayer({}, TileLayer::Type::Wall, wallSet,
                                             wallType);
            }
        }
        for (const auto& [tilePosition, tile] : tiles) {
            collisionLocator.updateTile(tilePosition, tile);
        }

        navGrid.setGridSize(extent);
        navGrid.build();
    }

    NavNode node(int x, int y, int z) { return navGrid.getNode({x, y, z}); }
};
} // namespace

TEST_CASE("NavNode bit packing")
{
    NavNode node{};
    REQUIRE(!(node.isStandable()));

    node.setSurfaceHeight(2);
    node.setClearance(5);
    node.setHeadroom(7);
    node.setWestBlocked(true);
    node.setNorthBlocked(false);
    REQUIRE(node.getSurfaceHeight() == 2);
    REQUIRE(node.getClearance() == 5);
    REQUIRE(node.getHeadroom() == 7);
    REQUIRE(node.isWestBlocked());
    REQUIRE(!(node.isNorthBlocked()));
    REQUIRE(node.isStandable());

    // Changing one field shouldn't affect the others.
    node.setClearance(0);
    node.setWestBlocked(false);
    node.setNorthBlocked(true);
    REQUIRE(node.getSurfaceHeight() == 2);
    REQUIRE(node.getClearance() == 0);
    REQUIRE(node.getHeadroom() == 7);
    REQUIRE(!(node.isWestBlocked()));
    REQUIRE(node.isNorthBlocked());
    REQUIRE(!(node.isStandable()));
}

TEST_CASE_METHOD(TestMap, "NavGrid flat ground")
{
    fillTerrain(0, Terrain::Height::Flat);
    build();

    NavNode origin{node(0, 0, 0)};
    REQUIRE(origin.isStandable());
    REQUIRE(origin.getSurfaceHeight() == 0);
    REQUIRE(origin.getHeadroom() == NavNode::MAX_HEADROOM);
    REQUIRE(!(origin.isWestBlocked()));
    REQUIRE(!(origin.isNorthBlocked()));

    // Nothing to stand on above the ground.
    REQUIRE(!(node(0, 0, 1).isStandable()));

    // Clearance grows away from the east and south edges of the grid, up to
    // the max.
    REQUIRE(node(15, 15, 0).getClearance() == 1);
    REQUIRE(node(14, 0, 0).getClearance() == 2);
    REQUIRE(node(13, 13, 0).getClearance() == 3);
    REQUIRE(node(-16, -16, 0).getClearance() == NavNode::MAX_CLEARANCE);

    // Out of bounds nodes are empty.
    REQUIRE(navGrid.getNode({100, 0, 0}) == NavNode{});
}

TEST_CASE_METHOD(TestMap, "NavGrid terrain heights")
{
    fillTerrain(0, Terrain::Height::Flat);
    addTerrain({0, 0, 0}, Terrain::Height::OneThird);
    addTerrain({1, 0, 0}, Terrain::Height::TwoThird);
    addTerrain({2, 0, 0}, Terrain::Height::Full);
    build();

    REQUIRE(node(0, 0, 0).getSurfaceHeight() == 1);
    REQUIRE(node(1, 0, 0).getSurfaceHeight() == 2);

    // Full terrain's surface is stored in the tile above it.
    REQUIRE(!(node(2, 0, 0).isStandable()));
    REQUIRE(node(2, 0, 1).isStandable());
    REQUIRE(node(2, 0, 1).getSurfaceHeight() == 0);
    REQUIRE(node(2, 0, 1).getHeadroom() == NavNode::MAX_HEADROOM);

    // Squares can't span multiple heights.
    REQUIRE(node(-1, -1, 0).getClearance() == 1);
    REQUIRE(node(-2, -2, 0).getClearance() == 2);
}

TEST_CASE_METHOD(TestMap, "NavGrid headroom")
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

    REQUIRE(node(0, 0, 0).getHeadroom() == 3);

    // The floating terrain limits the headroom below it, and provides a
    // surface on top of it.
    REQUIRE(node(1, 0, 0).getSurfaceHeight() == 0);
    REQUIRE(node(1, 0, 0).getHeadroom() == 4);
    REQUIRE(node(1, 0, 1).isStandable());
    REQUIRE(node(1, 0, 1).getSurfaceHeight() == 2);

    REQUIRE(!(node(2, 0, 0).isStandable()));
    REQUIRE(node(3, 0, 0).getHeadroom() == 1);
    REQUIRE(node(4, 0, 0).getHeadroom() == NavNode::MAX_HEADROOM);
}

TEST_CASE_METHOD(TestMap, "NavGrid walls")
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
    NavNode westWallNode{node(0, 0, 0)};
    REQUIRE(westWallNode.isStandable());
    REQUIRE(westWallNode.getHeadroom() == NavNode::MAX_HEADROOM);
    REQUIRE(westWallNode.isWestBlocked());
    REQUIRE(!(westWallNode.isNorthBlocked()));
    REQUIRE(!(node(-1, 0, 0).isWestBlocked()));

    NavNode northWallNode{node(0, 2, 0)};
    REQUIRE(northWallNode.isNorthBlocked());
    REQUIRE(!(northWallNode.isWestBlocked()));

    // A corner column only covers a small part of each edge.
    NavNode northWestGapFillNode{node(0, 4, 0)};
    REQUIRE(northWestGapFillNode.isStandable());
    REQUIRE(!(northWestGapFillNode.isWestBlocked()));
    REQUIRE(!(northWestGapFillNode.isNorthBlocked()));

    // A 3/4 north wall covers most of the north edge.
    NavNode northEastGapFillNode{node(0, 6, 0)};
    REQUIRE(northEastGapFillNode.isNorthBlocked());
    REQUIRE(!(northEastGapFillNode.isWestBlocked()));

    REQUIRE(node(0, 8, 0).getSurfaceHeight() == 1);
    REQUIRE(node(0, 8, 0).isWestBlocked());

    // Squares can't contain blocked edges.
    REQUIRE(node(-1, 0, 0).getClearance() == 1);
    REQUIRE(node(-1, -1, 0).getClearance() == 1);
    REQUIRE(node(0, 1, 0).getClearance() == 1);
    REQUIRE(node(0, 0, 0).getClearance() == 2);
}

TEST_CASE_METHOD(TestMap, "NavGrid obstacles")
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

    REQUIRE(!(node(0, 0, 0).isStandable()));

    REQUIRE(node(2, 0, 0).isStandable());
    REQUIRE(node(2, 0, 0).isWestBlocked());

    REQUIRE(!(node(4, 0, 0).isStandable()));

    // Neighbors are unaffected.
    REQUIRE(node(1, 0, 0).isStandable());
    REQUIRE(!(node(1, 0, 0).isWestBlocked()));
}
