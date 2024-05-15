#pragma once

#include "GraphicDataBase.h"
#include "Tile.h"
#include "GraphicSets.h"
#include "Rotation.h"
#include "Wall.h"
#include "Chunk.h"
#include "ChunkExtent.h"
#include "ChunkPosition.h"
#include "TilePosition.h"
#include "TileExtent.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "TileSnapshot.h"
#include "TileLayer.h"
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
    TileMapBase(GraphicDataBase& inGraphicData, bool inTrackTileUpdates);

    /**
     * Sets the given tile's floor to the given floor.
     * Note: Floor graphic sets only have 1 graphic, so you don't need to 
     *       specify which graphic from the set to use.
     */
    void setFloor(const TilePosition& tilePosition,
                  const FloorGraphicSet& graphicSet);
    void setFloor(const TilePosition& tilePosition,
                  const std::string& graphicSetID);
    void setFloor(const TilePosition& tilePosition, Uint16 graphicSetID);

    /**
     * Removes the given tile's floor. Since tiles only have 1 floor, this is
     * equivalent to clearTileLayers<FloorTileLayer>().
     */
    bool remFloor(const TilePosition& tilePosition);

    /**
     * Adds the given floor covering to the given tile.
     */
    void addFloorCovering(const TilePosition& tilePosition,
                          const FloorCoveringGraphicSet& graphicSet,
                          Rotation::Direction rotation);
    void addFloorCovering(const TilePosition& tilePosition,
                          const std::string& graphicSetID,
                          Rotation::Direction rotation);
    void addFloorCovering(const TilePosition& tilePosition, Uint16 graphicSetID,
                          Rotation::Direction rotation);

    /**
     * Removes the given floor covering from the given tile.
     * @return true if the tile had a floor covering to remove, else false.
     */
    bool remFloorCovering(const TilePosition& tilePosition,
                          const FloorCoveringGraphicSet& graphicSet,
                          Rotation::Direction rotation);
    bool remFloorCovering(const TilePosition& tilePosition,
                          const std::string& graphicSetID,
                          Rotation::Direction rotation);
    bool remFloorCovering(const TilePosition& tilePosition, Uint16 graphicSetID,
                          Rotation::Direction rotation);

    /**
     * Adds the given wall to the given tile.
     * Note: wallType must be North or West. Gap fills will be added
     *       automatically.
     */
    void addWall(const TilePosition& tilePosition,
                 const WallGraphicSet& graphicSet, Wall::Type wallType);
    void addWall(const TilePosition& tilePosition,
                 const std::string& graphicSetID, Wall::Type wallType);
    void addWall(const TilePosition& tilePosition, Uint16 graphicSetID,
                 Wall::Type wallType);

    /**
     * Removes the given wall from the given tile.
     * Note: wallType must be North or West. Gap fills will be removed
     *       automatically.
     * @return true if the tile had a wall to remove, else false.
     */
    bool remWall(const TilePosition& tilePosition, Wall::Type wallType);

    /**
     * Adds the given object to the given tile.
     */
    void addObject(const TilePosition& tilePosition,
                   const ObjectGraphicSet& graphicSet,
                   Rotation::Direction rotation);
    void addObject(const TilePosition& tilePosition,
                   const std::string& graphicSetID,
                   Rotation::Direction rotation);
    void addObject(const TilePosition& tilePosition, Uint16 graphicSetID,
                   Rotation::Direction rotation);

    /**
     * Removes the given object from the given tile.
     * @return true if the tile had an object to remove, else false.
     */
    bool remObject(const TilePosition& tilePosition,
                   const ObjectGraphicSet& graphicSet,
                   Rotation::Direction rotation);
    bool remObject(const TilePosition& tilePosition,
                   const std::string& graphicSetID,
                   Rotation::Direction rotation);
    bool remObject(const TilePosition& tilePosition, Uint16 graphicSetID,
                   Rotation::Direction rotation);

    /**
     * Clears the given layer types from the given tile.
     *
     * @param layerTypesToClear The layer types to clear.
     * @return true if any layers were cleared. false if the tile was empty.
     */
    bool clearTileLayers(
        const TilePosition& tilePosition,
        const std::initializer_list<TileLayer::Type>& layerTypesToClear);

    /**
     * Override for clearing based on an array of bools. If a given index
     * is true, the associated TileLayer::Type will be cleared.
     * You probably don't want to use this, it's mostly useful for messaging.
     */
    bool clearTileLayers(
        const TilePosition& tilePosition,
        const std::array<bool, TileLayer::Type::Count>& layerTypesToClear);

    /**
     * Clears all layers from the given tile.
     * @return true if any layers were cleared. false if the tile was empty.
     */
    bool clearTile(const TilePosition& tilePosition);

    /**
     * Clears the given layer types from all tiles within the given extent.
     *
     * @param layerTypesToClear The layer types to clear in each tile.
     * @return true if any layers were cleared. false if all tiles were empty.
     */
    bool clearExtentLayers(
        const TileExtent& extent,
        const std::initializer_list<TileLayer::Type>& layerTypesToClear);

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
     * Gets a const reference to the chunk at the given coordinates.
     */
    const Chunk& getChunk(const ChunkPosition& chunkPosition) const;

    /**
     * Gets a const reference to the tile at the given coordinates.
     */
    const Tile& getTile(const TilePosition& tilePosition) const;
    /** This lets us call getTile on a non-const TileMap& without casting. */
    const Tile& cgetTile(const TilePosition& tilePosition) const;

    /**
     * Returns the map extent, with chunks as the unit.
     */
    const ChunkExtent& getChunkExtent() const;

    /**
     * Returns the map extent, with tiles as the unit.
     */
    const TileExtent& getTileExtent() const;

    /**
     * If true, when a tile is updated, its collision will be rebuilt.
     *
     * If false, the user must manually call rebuildDirtyTileCollision() after 
     * finishing all of their tile updates.
     *
     * When set from false to true, rebuildDirtyTileCollision() is called.
     */
    void setAutoRebuildCollision(bool newAutoRebuildCollision);

    /**
     * Rebuilds the collision of any tiles that have been updated since the 
     * last time this was called (while autoRebuildCollision is disabled).
     *
     * You normally don't need to call this manually, since it's called when 
     * autoRebuildCollision is re-enabled.
     */
    void rebuildDirtyTileCollision();

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
     * Adds the graphic layers from the given snapshot to the given tile.
     */
    template<IsChunkSnapshotType T>
    void addSnapshotLayersToTile(const TileSnapshot& tileSnapshot,
                                 const T& chunkSnapshot,
                                 const TilePosition& tilePosition);

protected:
    /**
     * Returns the index in the chunks vector where the chunk with the given
     * coordinates can be found.
     */
    inline std::size_t
        linearizeChunkIndex(const ChunkPosition& chunkPosition) const
    {
        // Translate the given position from actual-space to positive-space.
        ChunkPosition positivePosition{chunkPosition.x - chunkExtent.x,
                                       chunkPosition.y - chunkExtent.y,
                                       chunkPosition.z - chunkExtent.z};

        return static_cast<std::size_t>(
            (chunkExtent.xLength * chunkExtent.yLength * positivePosition.z)
            + (chunkExtent.xLength * positivePosition.y) + positivePosition.x);
    }

    /**
     * Gets a reference to the tile at the given coordinates.
     */
    Tile& getTile(const TilePosition& tilePosition);

    /**
     * If auto rebuild is enabled, rebuilds the given tile's collision.
     * Otherwise, queues the collision to be rebuilt.
     */
    void rebuildTileCollision(Tile& tile, const TilePosition& tilePosition);

    /**
     * Adds a North wall to the given tile and adds gap fills if necessary.
     */
    void addNorthWall(const TilePosition& tilePosition,
                      const WallGraphicSet& graphicSet);

    /**
     * Adds a West wall to the given tile and adds gap fills if necessary.
     */
    void addWestWall(const TilePosition& tilePosition,
                     const WallGraphicSet& graphicSet);

    /**
     * Removes the North wall from the given tile. If a corner was broken,
     * modifies the other wall pieces appropriately.
     */
    bool remNorthWall(const TilePosition& tilePosition);

    /**
     * Removes the West wall from the given tile. If a corner was broken,
     * modifies the other wall pieces appropriately.
     */
    bool remWestWall(const TilePosition& tilePosition);

    /**
     * Clears the given layer types from the given tile.
     */
    bool clearTileLayersInternal(
        const TilePosition& tilePosition,
        const std::array<bool, TileLayer::Type::Count>& layerTypesToClear);

    /**
     * Returns a bool array of layer types to clear, based on the given
     * list of type enums.
     * We do things this way because it's more convenient for the caller to 
     * pass a list of type enums than an array of bools.
     */
    std::array<bool, TileLayer::Type::Count> toBoolArray(
        const std::initializer_list<TileLayer::Type>& layerTypesToClear);

    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    static constexpr Uint16 MAP_FORMAT_VERSION{1};

    /** Used to get graphics while constructing tiles. */
    GraphicDataBase& graphicData;

    /** The map's extent, with chunks as the unit. */
    ChunkExtent chunkExtent;

    /** The map's extent, with tiles as the unit. */
    TileExtent tileExtent;

    /** The chunks that make up this map, stored in row-major order. */
    std::vector<Chunk> chunks;

private:
    /**
     * If true, collision will be rebuilt every time a tile is modified.
     * If false, the user must manually call rebuildDirtyTileCollision().
     */
    bool autoRebuildCollision;

    /** A queue of tiles that need their collision rebuilt. */
    std::vector<TilePosition> dirtyCollisionQueue;

    /** If true, all tile updates will be pushed into tileUpdateHistory. */
    bool trackTileUpdates;

    /** Holds a history of tile operations that have been performed on this map.
        TileUpdateSystem uses this history to send updates to clients, then
        clears it. */
    std::vector<TileUpdateVariant> tileUpdateHistory;
};

template<IsChunkSnapshotType T>
void TileMapBase::addSnapshotLayersToTile(const TileSnapshot& tileSnapshot,
                                          const T& chunkSnapshot,
                                          const TilePosition& tilePosition)
{
    // Note: We can't use the set/add functions because they'll push updates
    //       into the history, and addWall() adds extra walls.

    // Iterate the tile snapshot and add each tile layer.
    bool rebuildCollision{false};
    Tile& tile{getTile(tilePosition)};
    for (Uint8 paletteIndex : tileSnapshot.layers) {
        const auto& paletteEntry{chunkSnapshot.palette[paletteIndex]};

        // Get this layer's graphic set.
        const GraphicSet* graphicSet{nullptr};
        switch (paletteEntry.layerType) {
            case TileLayer::Type::Floor: {
                graphicSet = &(
                    graphicData.getFloorGraphicSet(paletteEntry.graphicSetID));
                rebuildCollision = true;
                break;
            }
            case TileLayer::Type::FloorCovering: {
                graphicSet = &(graphicData.getFloorCoveringGraphicSet(
                    paletteEntry.graphicSetID));
                break;
            }
            case TileLayer::Type::Wall: {
                graphicSet = &(
                    graphicData.getWallGraphicSet(paletteEntry.graphicSetID));
                rebuildCollision = true;
                break;
            }
            case TileLayer::Type::Object: {
                graphicSet = &(
                    graphicData.getObjectGraphicSet(paletteEntry.graphicSetID));
                rebuildCollision = true;
                break;
            }
            default: {
                break;
            }
        }

        if (!graphicSet) {
            LOG_FATAL("Graphic set was not found for loaded tile layer.");
        }

        // Add the layer to the tile.
        tile.addLayer(paletteEntry.layerType, *graphicSet,
                      paletteEntry.graphicIndex);
    }

    if (rebuildCollision) {
        dirtyCollisionQueue.emplace_back(tilePosition);
    }
}

} // End namespace AM
