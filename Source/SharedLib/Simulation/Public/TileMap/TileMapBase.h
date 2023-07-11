#pragma once

#include "SpriteDataBase.h"
#include "Tile.h"
#include "SpriteSets.h"
#include "Rotation.h"
#include "Wall.h"
#include "ChunkExtent.h"
#include "TileExtent.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "TileSnapshot.h"
#include "TileLayers.h"
#include "AMAssert.h"
#include <vector>
#include <variant>
#include <type_traits>

/** Concept to match ChunkSnapshot and ChunkWireSnapshot. */
template<typename T>
concept IsChunkSnapshotType = requires(T a)
{
    a.palette;
    a.tiles;
};

namespace AM
{
struct TileMapSnapshot;

/**
 * Owns and manages the world's tile map state.
 * Tiles are conceptually organized into 16x16 chunks.
 *
 * Persisted tile map data is loaded from TileMap.bin.
 */
class TileMapBase
{
public:
    /**
     * Attempts to parse TileMap.bin and construct the tile map.
     *
     * Errors if TileMap.bin doesn't exist or it fails to parse.
     *
     * @param inTrackTileUpdates  If true, tile updates will be pushed into 
     *                            tileUpdateHistory.
     */
    TileMapBase(SpriteDataBase& inSpriteData, bool inTrackTileUpdates);

    /**
     * Sets the given tile's floor to the given floor.
     * Note: Floor sprite sets only have 1 sprite, so you don't need to specify 
     *       which sprite from the set to use.
     */
    void setFloor(int tileX, int tileY, const FloorSpriteSet& spriteSet);
    void setFloor(int tileX, int tileY, const std::string& spriteSetID);
    void setFloor(int tileX, int tileY, Uint16 spriteSetID);

    /**
     * Removes the given tile's floor. Since tiles only have 1 floor, this is 
     * equivalent to clearTileLayers<FloorTileLayer>().
     */
    bool remFloor(int tileX, int tileY);

    /**
     * Adds the given floor covering to the given tile.
     */
    void addFloorCovering(int tileX, int tileY, const FloorCoveringSpriteSet& spriteSet,
                          Rotation::Direction rotation);
    void addFloorCovering(int tileX, int tileY, const std::string& spriteSetID,
                          Rotation::Direction rotation);
    void addFloorCovering(int tileX, int tileY, Uint16 spriteSetID,
                          Rotation::Direction rotation);

    /**
     * Removes the given floor covering from the given tile.
     * @return true if the tile had a floor covering to remove, else false.
     */
    bool remFloorCovering(int tileX, int tileY, const FloorCoveringSpriteSet& spriteSet,
                          Rotation::Direction rotation);
    bool remFloorCovering(int tileX, int tileY, const std::string& spriteSetID,
                          Rotation::Direction rotation);
    bool remFloorCovering(int tileX, int tileY, Uint16 spriteSetID,
                          Rotation::Direction rotation);

    /**
     * Adds the given wall to the given tile.
     * Note: wallType must be North or West. Gap fills will be added 
     *       automatically.
     */
    void addWall(int tileX, int tileY, const WallSpriteSet& spriteSet,
                          Wall::Type wallType);
    void addWall(int tileX, int tileY, const std::string& spriteSetID,
                          Wall::Type wallType);
    void addWall(int tileX, int tileY, Uint16 spriteSetID, Wall::Type wallType);

    /**
     * Removes the given wall from the given tile.
     * Note: wallType must be North or West. Gap fills will be removed 
     *       automatically.
     * @return true if the tile had a wall to remove, else false.
     */
    bool remWall(int tileX, int tileY, Wall::Type wallType);

    /**
     * Adds the given object to the given tile.
     */
    void addObject(int tileX, int tileY, const ObjectSpriteSet& spriteSet,
                          Rotation::Direction rotation);
    void addObject(int tileX, int tileY, const std::string& spriteSetID,
                          Rotation::Direction rotation);
    void addObject(int tileX, int tileY, Uint16 spriteSetID,
                          Rotation::Direction rotation);

    /**
     * Removes the given object from the given tile.
     * @return true if the tile had an object to remove, else false.
     */
    bool remObject(int tileX, int tileY, const ObjectSpriteSet& spriteSet,
                          Rotation::Direction rotation);
    bool remObject(int tileX, int tileY, const std::string& spriteSetID,
                          Rotation::Direction rotation);
    bool remObject(int tileX, int tileY, Uint16 spriteSetID,
                          Rotation::Direction rotation);

    /**
     * Clears the given layer types from the given tile.
     *
     * @tparam LayersToClear  The layer types to clear.
     * @return true if any layers were cleared. false if the tile was empty.
     */
    template<IsTileLayerType... LayersToClear>
    bool clearTileLayers(int tileX, int tileY);

    /**
     * Override for clearing based on an array of bools. If a given index 
     * is true, the associated TileLayer::Type will be cleared.
     * You probably don't want to use this, it's mostly useful for messaging.
     */
    bool clearTileLayers(
        int tileX, int tileY,
        const std::array<bool, TileLayer::Type::Count>& layerTypesToClear);

    /**
     * Clears all layers from the given tile.
     * @return true if any layers were cleared. false if the tile was empty.
     */
    bool clearTile(int tileX, int tileY);

    /**
     * Clears the given layer types from all tiles within the given extent.
     *
     * @tparam LayersToClear  The layer types to clear in each tile.
     * @return true if any layers were cleared. false if all tiles were empty.
     */
    template<IsTileLayerType... LayersToClear>
    bool clearExtentLayers(const TileExtent& extent);

    /**
     * Override for clearing based on an array of bools. If a given index 
     * is true, the associated TileLayer::Type will be cleared.
     * You probably don't want to use this, it's mostly useful for messaging.
     */
    bool clearExtentLayers(
        const TileExtent& extent,
        const std::array<bool, TileLayer::Type::Count>& layerTypesToClear);

    /**
     * Clears all layers from all tiles within the given extent.
     * @return true if any layers were cleared. false if all tiles were empty.
     */
    bool clearExtent(const TileExtent& extent);

    /**
     * Clears all tile map state, leaving an empty map.
     */
    void clear();

    /**
     * Gets a const reference to the tile at the given coordinates.
     */
    const Tile& getTile(int tileX, int tileY) const;

    /**
     * Returns the map extent, with chunks as the unit.
     */
    const ChunkExtent& getChunkExtent() const;

    /**
     * Returns the map extent, with tiles as the unit.
     */
    const TileExtent& getTileExtent() const;

    using TileUpdateVariant
        = std::variant<TileAddLayer, TileRemoveLayer, TileClearLayers,
                       TileExtentClearLayers>;
    /**
     * Returns a vector containing all operations that have been performed on 
     * this tile map since the last time the vector was cleared.
     */
    const std::vector<TileUpdateVariant>& getTileUpdateHistory();

    /**
     * Clears the tile update history vector.
     */
    void clearTileUpdateHistory();

    /**
     * Adds the sprite layers from the given snapshot to the given tile.
     */
    template<IsChunkSnapshotType T>
    void addSnapshotLayersToTile(const TileSnapshot& tileSnapshot,
                                 const T& chunkSnapshot,
                                 int tileX, int tileY);

protected:
    /**
     * Returns the index in the tiles vector where the tile with the given
     * coordinates can be found.
     */
    inline std::size_t linearizeTileIndex(int x, int y) const
    {
        return static_cast<std::size_t>((y * tileExtent.xLength) + x);
    }

    /**
     * Adds a North wall to the given tile and adds gap fills if necessary.
     */
    void addNorthWall(int tileX, int tileY, const WallSpriteSet& spriteSet);

    /**
     * Adds a West wall to the given tile and adds gap fills if necessary.
     */
    void addWestWall(int tileX, int tileY, const WallSpriteSet& spriteSet);

    /**
     * Removes the North wall from the given tile. If a corner was broken, 
     * modifies the other wall pieces appropriately.
     */
    bool remNorthWall(int tileX, int tileY);

    /**
     * Removes the West wall from the given tile. If a corner was broken, 
     * modifies the other wall pieces appropriately.
     */
    bool remWestWall(int tileX, int tileY);

    /**
     * Clears the given layer types from the given tile.
     */
    bool clearTileLayersInternal(
        int tileX, int tileY,
        const std::array<bool, TileLayer::Type::Count>& layerTypesToClear);

    /**
     * Returns an array of layer types to clear, based on the given 
     * LayersToClear.
     */
    template<IsTileLayerType... LayersToClear>
    std::array<bool, TileLayer::Type::Count> getLayerTypesToClear();

    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    static constexpr Uint16 MAP_FORMAT_VERSION{1};

    /** Used to get sprites while constructing tiles. */
    SpriteDataBase& spriteData;

    /** The map's extent, with chunks as the unit. */
    ChunkExtent chunkExtent;

    /** The map's extent, with tiles as the unit. */
    TileExtent tileExtent;

    /** The tiles that make up this map, stored in row-major order. */
    std::vector<Tile> tiles;

private:
    /** If true, all tile updates will be pushed into tileUpdateHistory. */
    bool trackTileUpdates;

    /** Holds a history of tile operations that have been performed on this map.
        TileUpdateSystem uses this history to send updates to clients, then 
        clears it. */
    std::vector<TileUpdateVariant> tileUpdateHistory;
};

template<IsTileLayerType... LayersToClear>
bool TileMapBase::clearTileLayers(int tileX, int tileY)
{
    return clearTileLayers(tileX, tileY,
                           getLayerTypesToClear<LayersToClear...>());
}

template<IsTileLayerType... LayersToClear>
bool TileMapBase::clearExtentLayers(const TileExtent& extent)
{
    return clearExtentLayers(extent, getLayerTypesToClear<LayersToClear...>());
}

template<IsChunkSnapshotType T>
void TileMapBase::addSnapshotLayersToTile(const TileSnapshot& tileSnapshot,
                             const T& chunkSnapshot,
                             int tileX, int tileY)
{
    // Note: We can't use the set/add functions because they'll push updates 
    //       into the history, and addWall() adds extra walls.
    Tile& tile{tiles[linearizeTileIndex(tileX, tileY)]};
    for (Uint8 paletteIndex : tileSnapshot.layers) {
        const auto& paletteEntry{chunkSnapshot.palette[paletteIndex]};

        switch (paletteEntry.layerType) {
            case TileLayer::Type::Floor: {
                const FloorSpriteSet& spriteSet{
                    spriteData.getFloorSpriteSet(paletteEntry.spriteSetID)};
                tile.getFloor().spriteSet = &spriteSet;
                break;
            }
            case TileLayer::Type::FloorCovering: {
                const auto& floorCoverings{tile.getFloorCoverings()};
                const auto& spriteSet{spriteData.getFloorCoveringSpriteSet(
                    paletteEntry.spriteSetID)};
                tile.getFloorCoverings().emplace_back(
                    &spriteSet,
                    static_cast<Rotation::Direction>(paletteEntry.spriteIndex));
                break;
            }
            case TileLayer::Type::Wall: {
                std::array<WallTileLayer, 2>& walls{tile.getWalls()};
                const WallSpriteSet& spriteSet{
                    spriteData.getWallSpriteSet(paletteEntry.spriteSetID)};

                Wall::Type newWallType{
                    static_cast<Wall::Type>(paletteEntry.spriteIndex)};
                if (newWallType == Wall::Type::West) {
                    walls[0].spriteSet = &spriteSet;
                    walls[0].wallType = newWallType;
                }
                else {
                    walls[1].spriteSet = &spriteSet;
                    walls[1].wallType = newWallType;
                }
                tile.rebuildCollision(tileX, tileY);
                break;
            }
            case TileLayer::Type::Object: {
                const std::vector<ObjectTileLayer>& objects{tile.getObjects()};
                const auto& spriteSet{
                    spriteData.getObjectSpriteSet(paletteEntry.spriteSetID)};
                tile.getObjects().emplace_back(
                    &spriteSet,
                    static_cast<Rotation::Direction>(paletteEntry.spriteIndex));
                tile.rebuildCollision(tileX, tileY);
                break;
            }
        }
    }
}

template<IsTileLayerType... LayersToClear>
std::array<bool, TileLayer::Type::Count> TileMapBase::getLayerTypesToClear()
{
    std::array<bool, TileLayer::Type::Count> layerTypesToClear{};
    if constexpr ((std::is_same_v<FloorTileLayer, LayersToClear> || ...)) {
        layerTypesToClear[TileLayer::Type::Floor] = true;
    }

    if constexpr ((std::is_same_v<FloorCoveringTileLayer,
                                 LayersToClear> || ...)) {
        layerTypesToClear[TileLayer::Type::FloorCovering] = true;
    }

    if constexpr ((std::is_same_v<WallTileLayer, LayersToClear> || ...)) {
        layerTypesToClear[TileLayer::Type::Wall] = true;
    }

    if constexpr ((std::is_same_v<ObjectTileLayer, LayersToClear> || ...)) {
        layerTypesToClear[TileLayer::Type::Object] = true;
    }

    return layerTypesToClear;
}

} // End namespace AM
