#pragma once

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
#include "catch2/catch_all.hpp"
#include <unordered_map>
#include <span>
#include <algorithm>
#include <unordered_set>
#include <vector>

namespace AM
{
/**
 * A small test map: 2x2 chunks centered on the origin, 4 tiles tall.
 *
 * Tile layers and static entities are added directly to a collision locator,
 * then the nav grid is built from it.
 */
struct NavTestMap {
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

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
    /** Tiles that have changed since they were last added to the locator. */
    std::unordered_set<TilePosition> dirtyTiles{};
    Uint32 nextEntityID{0};

    CollisionLocator collisionLocator{};
    NavGrid navGrid{collisionLocator};

    NavTestMap() { collisionLocator.setGridSize(extent); }

    NavTestMap(const TileExtent& inExtent)
    : extent{inExtent}
    {
        collisionLocator.setGridSize(extent);
    }

    void addTerrain(const TilePosition& tilePosition, Terrain::Height height,
                    Terrain::Height startHeight = Terrain::Height::Flat)
    {
        terrain[tilePosition] = Terrain::toValue(height, startHeight);
        dirtyTiles.insert(tilePosition);
    }

    void remTerrain(const TilePosition& tilePosition)
    {
        terrain.erase(tilePosition);
        dirtyTiles.insert(tilePosition);
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
        dirtyTiles.insert(tilePosition);
    }

    void remWalls(const TilePosition& tilePosition)
    {
        walls.erase(tilePosition);
        dirtyTiles.insert(tilePosition);
    }

    /** Adds an entity with the given world-space collision. */
    entt::entity addEntity(const BoundingBox& worldBounds,
                           CollisionLayerBitSet collisionLayers
                           = CollisionLayerType::BlockCollision)
    {
        entt::entity entity{static_cast<entt::entity>(nextEntityID++)};
        collisionLocator.updateEntity(entity, worldBounds, collisionLayers);
        return entity;
    }

    /** Adds an entity within the given tile, offset by the given bounds. */
    entt::entity addTileEntity(const TilePosition& tilePosition,
                               const BoundingBox& tileBounds,
                               CollisionLayerBitSet collisionLayers
                               = CollisionLayerType::BlockCollision)
    {
        return addEntity(tileBounds.translateBy(tilePosition.getOriginPoint()),
                         collisionLayers);
    }

    /** Moves an entity to the given tile, offset by the given bounds. */
    void moveTileEntity(entt::entity entity, const TilePosition& tilePosition,
                        const BoundingBox& tileBounds,
                        CollisionLayerBitSet collisionLayers
                        = CollisionLayerType::BlockCollision)
    {
        collisionLocator.updateEntity(
            entity, tileBounds.translateBy(tilePosition.getOriginPoint()),
            collisionLayers);
    }

    /** Adds the tiles to the collision locator and builds the nav grid. */
    void build()
    {
        syncTiles();
        navGrid.setGridSize(extent);
        navGrid.build();
    }

    /** Adds any tile changes to the collision locator and updates the nav
        grid. */
    void update()
    {
        syncTiles();
        navGrid.update();
    }

    /** Adds any changed tiles to the collision locator. */
    void syncTiles()
    {
        for (const TilePosition& tilePosition : dirtyTiles) {
            Tile tile{};
            if (auto terrainIt{terrain.find(tilePosition)};
                terrainIt != terrain.end()) {
                tile.addLayer({}, TileLayer::Type::Terrain, terrainSet,
                              terrainIt->second);
            }
            if (auto wallsIt{walls.find(tilePosition)};
                wallsIt != walls.end()) {
                for (Wall::Type wallType : wallsIt->second) {
                    tile.addLayer({}, TileLayer::Type::Wall, wallSet, wallType);
                }
            }
            collisionLocator.updateTile(tilePosition, tile);
        }
        dirtyTiles.clear();
    }

    /** Requires that the nav grid matches a freshly built one. */
    void requireMatchesFullBuild()
    {
        NavGrid fullGrid{collisionLocator};
        fullGrid.setGridSize(extent);
        fullGrid.build();

        for (int y{extent.y}; y <= extent.yMax(); ++y) {
            for (int x{extent.x}; x <= extent.xMax(); ++x) {
                std::span<const NavSurface> updatedSurfaces{
                    navGrid.getSurfaces(x, y)};
                std::span<const NavSurface> builtSurfaces{
                    fullGrid.getSurfaces(x, y)};
                if (!std::ranges::equal(updatedSurfaces, builtSurfaces)) {
                    INFO("Mismatch at (" << x << ", " << y << ")");
                    REQUIRE(updatedSurfaces.size() == builtSurfaces.size());
                    for (std::size_t i{0}; i < builtSurfaces.size(); ++i) {
                        REQUIRE(updatedSurfaces[i].height
                                == builtSurfaces[i].height);
                        REQUIRE(updatedSurfaces[i].headroom
                                == builtSurfaces[i].headroom);
                        REQUIRE(updatedSurfaces[i].flags
                                == builtSurfaces[i].flags);
                    }
                }
            }
        }
    }

    /**
     * The highest surface within a single tile, if it has one.
     */
    struct TileSurface {
        const NavSurface* surface{nullptr};
        /** The surface's height, relative to the bottom of the tile. */
        int tileHeight{0};

        bool isStandable() const { return (surface != nullptr); }
        int getSurfaceHeight() const { return tileHeight; }
        int getHeadroom() const { return surface->headroom; }
        bool isWestBlocked() const { return surface->isWestBlocked(); }
        bool isNorthBlocked() const { return surface->isNorthBlocked(); }
    };

    /** Returns the highest surface within the given tile. */
    TileSurface tileSurface(int x, int y, int z)
    {
        TileSurface tileSurface{};
        int tileMinHeight{(z - extent.z) * 3};
        for (const NavSurface& surface : navGrid.getSurfaces(x, y)) {
            if ((surface.height >= tileMinHeight)
                && (surface.height < (tileMinHeight + 3))) {
                tileSurface.surface = &surface;
                tileSurface.tileHeight = surface.height - tileMinHeight;
            }
        }
        return tileSurface;
    }

    /** The headroom of a surface at the given height, with nothing above it
        (it's limited by the top of the map). */
    int openHeadroom(int height) const { return (extent.zLength * 3) - height; }
};

} // End namespace AM
