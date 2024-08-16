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
#include "TileLayer.h"
#include "Morton.h"
#include "AMAssert.h"
#include <vector>
#include <unordered_map>
#include <variant>
#include <type_traits>
#include <expected>

namespace AM
{
class CollisionLocator;
struct TileMapSnapshot;
struct ChunkSnapshot;
struct ChunkWireSnapshot;

/**
 * Owns and manages the world's tile map state.
 * Tiles are conceptually organized into 16x16 chunks.
 *
 * Persisted tile map data is loaded from TileMap.bin.
 *
 * When updating tiles, all of the following are handled by this class:
 *   1. If adding layers and the tile's parent chunk doesn't already exist, 
 *      creates it.
 *   2. Updates the tile's layers as requested.
 *   3. Rebuilds the tile's collision if necessary.
 *   4. Updates the parent chunk's nonEmptyTileCount if necessary.
 *   5. If removing layers and the parent chunk is now empty, erases it.
 *   6. Adds an entry to the tile update history, so update messages get sent
 *      to clients.
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
    TileMapBase(GraphicDataBase& inGraphicData,
                CollisionLocator& inCollisionLocator, bool inTrackTileUpdates);

    /**
     * Adds the given terrain to the given tile.
     */
    void addTerrain(const TilePosition& tilePosition,
                    const TerrainGraphicSet& graphicSet,
                    Terrain::Value terrainValue);
    void addTerrain(const TilePosition& tilePosition,
                    const std::string& graphicSetID,
                    Terrain::Value terrainValue);
    void addTerrain(const TilePosition& tilePosition, Uint16 graphicSetID,
                    Terrain::Value terrainValue);

    /**
     * Removes the terrain from the given tile.
     * @return true if the tile had terrain to remove, else false.
     */
    bool remTerrain(const TilePosition& tilePosition);

    /**
     * Adds the given floor to the given tile.
     */
    void addFloor(const TilePosition& tilePosition,
                  const TileOffset& tileOffset,
                  const FloorGraphicSet& graphicSet,
                  Rotation::Direction rotation);
    void addFloor(const TilePosition& tilePosition,
                  const TileOffset& tileOffset,
                  const std::string& graphicSetID,
                  Rotation::Direction rotation);
    void addFloor(const TilePosition& tilePosition,
                  const TileOffset& tileOffset, Uint16 graphicSetID,
                  Rotation::Direction rotation);

    /**
     * Removes the given object from the given tile.
     * @return true if the tile had an object to remove, else false.
     */
    bool remFloor(const TilePosition& tilePosition,
                  const TileOffset& tileOffset,
                  const FloorGraphicSet& graphicSet,
                  Rotation::Direction rotation);
    bool remFloor(const TilePosition& tilePosition,
                  const TileOffset& tileOffset, const std::string& graphicSetID,
                  Rotation::Direction rotation);
    bool remFloor(const TilePosition& tilePosition,
                  const TileOffset& tileOffset, Uint16 graphicSetID,
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
                   const TileOffset& tileOffset,
                   const ObjectGraphicSet& graphicSet,
                   Rotation::Direction rotation);
    void addObject(const TilePosition& tilePosition,
                   const TileOffset& tileOffset,
                   const std::string& graphicSetID,
                   Rotation::Direction rotation);
    void addObject(const TilePosition& tilePosition,
                   const TileOffset& tileOffset, Uint16 graphicSetID,
                   Rotation::Direction rotation);

    /**
     * Removes the given object from the given tile.
     * @return true if the tile had an object to remove, else false.
     */
    bool remObject(const TilePosition& tilePosition,
                   const TileOffset& tileOffset,
                   const ObjectGraphicSet& graphicSet,
                   Rotation::Direction rotation);
    bool remObject(const TilePosition& tilePosition,
                   const TileOffset& tileOffset,
                   const std::string& graphicSetID,
                   Rotation::Direction rotation);
    bool remObject(const TilePosition& tilePosition,
                   const TileOffset& tileOffset, Uint16 graphicSetID,
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
     * Returns a const pointer to the chunk at the given coordinates, or nullptr 
     * if the chunk doesn't exist (out of bounds, empty).
     * Note: Make sure to nullptr check these! Chunks are commonly empty. 
     */
    const Chunk* getChunk(const ChunkPosition& chunkPosition) const;
    /** This lets us call getChunk on a non-const TileMap& without casting. */
    const Chunk* cgetChunk(const ChunkPosition& chunkPosition) const;

    /**
     * Returns a const pointer to the tile at the given coordinates, or nullptr 
     * if the tile doesn't exist (out of bounds, parent chunk is empty).
     * Note: Make sure to nullptr check these! Chunks are commonly empty. 
     */
    const Tile* getTile(const TilePosition& tilePosition) const;
    const Tile* cgetTile(const TilePosition& tilePosition) const;

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
     * Adds the tile layers from the given chunk snapshot to the map.
     */
    void loadChunk(const ChunkSnapshot& chunkSnapshot,
                   const ChunkPosition& chunkPosition);
    void loadChunk(const ChunkWireSnapshot& chunkSnapshot,
                   const ChunkPosition& chunkPosition);

protected:
    enum class ChunkError {
        /** The given position was outside of the map bounds. */
        InvalidPosition,
        /** The requested chunk (or the parent chunk of the requested tile) 
            does not exist. */
        NotFound
    };

    /**
     * Returns a reference to the chunk at the given coordinates, or an 
     * appropriate error.
     */
    std::expected<std::reference_wrapper<Chunk>, ChunkError>
        getChunk(const ChunkPosition& chunkPosition);

    struct ChunkTilePair
    {
        /** The tile's parent chunk. */
        std::reference_wrapper<Chunk> chunk;
        std::reference_wrapper<Tile> tile;
    };
    /**
     * Returns a reference to the tile at the given coordinates, or an 
     * appropriate error.
     */
    std::expected<ChunkTilePair, ChunkError>
        getTile(const TilePosition& tilePosition);

    /**
     * Returns a reference to the tile at the given coordinates.
     * Note: The given chunk must contain the tile at the given position.
     */
    Tile& getTile(Chunk& chunk, const ChunkPosition& chunkPosition,
                  const TilePosition& tilePosition);

    /**
     * Returns a pointer to the chunk at the given coordinates, creating the 
     * chunk if it doesn't already exist.
     * If chunkPosition is outside of the map bounds, returns nullptr.
     */
    Chunk* getOrCreateChunk(const ChunkPosition& chunkPosition);

    struct ChunkTilePtrPair
    {
        /** The tile's parent chunk. */
        Chunk* chunk{nullptr};
        Tile* tile{nullptr};
    };
    /**
     * Returns a pointer to the chunk and tile at the given coordinates, creating
     * the parent chunk if it doesn't already exist.
     * If tilePosition is outside of the map bounds, returns {nullptr, nullptr}.
     */
    ChunkTilePtrPair getOrCreateTile(const TilePosition& tilePosition);

    /**
     * Adds the given layer to the specified tile.
     * @return The tile that was added, or nullptr (tilePosition was outside of 
     *         the map bounds).
     */
    Tile* addTileLayer(const TilePosition& tilePosition,
                       const TileOffset& tileOffset, TileLayer::Type layerType,
                       const GraphicSet& graphicSet, Uint8 graphicValue);

    /**
     * Adds the given layer to the given tile.
     */
    void addTileLayer(Chunk& chunk, Tile& tile, const TileOffset& tileOffset,
                      TileLayer::Type layerType, const GraphicSet& graphicSet,
                      Uint8 graphicValue);

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
     * Removes any layers with a matching offset, type, graphic index, and
     * graphic set from the specified tile.
     * @return The tile that was removed from, or nullptr (tilePosition was 
     *         outside of the map bounds, layer didn't exist).
     */
    Tile* remTileLayer(const TilePosition& tilePosition,
                       const TileOffset& tileOffset, TileLayer::Type layerType,
                       Uint16 graphicSetID, Uint8 graphicValue);

    /**
     * Removes any layers with a matching, type, graphic index, and graphic set
     * from the specified tile.
     * @return The tile that was removed from, or nullptr (tilePosition was 
     *         outside of the map bounds, layer didn't exist).
     */
    Tile* remTileLayer(const TilePosition& tilePosition,
                       TileLayer::Type layerType, Uint16 graphicSetID,
                       Uint8 graphicValue);

    /**
     * Removes any layers with a matching offset, type, graphic index, and
     * graphic set from the given tile.
     * @return true if the tile had any matching layers to remove, else false.
     */
    bool remTileLayer(Chunk& chunk, Tile& tile,
                      const ChunkPosition& chunkPosition,
                      const TileOffset& tileOffset, TileLayer::Type layerType,
                      Uint16 graphicSetID, Uint8 graphicValue);

    /**
     * Removes any layers with a matching type, graphic index, and graphic set
     * from the given tile.
     * @return true if the tile had any matching layers to remove, else false.
     */
    bool remTileLayer(Chunk& chunk, Tile& tile,
                      const ChunkPosition& chunkPosition,
                      TileLayer::Type layerType, Uint16 graphicSetID,
                      Uint8 graphicValue);

    /**
     * Removes any layers with a matching type and graphic index, regardless 
     * of their graphic set or offset.
     * @return true if the tile had any matching layers to remove, else false.
     */
    bool remTileLayers(Chunk& chunk, Tile& tile,
                       const ChunkPosition& chunkPosition,
                       TileLayer::Type layerType, Uint8 graphicValue);

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
     * @return If any layers were cleared, returns the tile. Else, nullptr 
     *         (tilePosition is outside of map bounds).
     */
    Tile* clearTileLayersInternal(
        const TilePosition& tilePosition,
        const std::array<bool, TileLayer::Type::Count>& layerTypesToClear);

    /**
     * Adds the tile layers from the given chunk snapshot to the map.
     */
    template<typename T>
    void loadChunkInternal(const T& chunkSnapshot,
                           const ChunkPosition& chunkPosition);

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

    /** Used when rebuilding tile collision. */
    CollisionLocator& collisionLocator;

    /** The map's extent, with chunks as the unit. */
    ChunkExtent chunkExtent;

    /** The map's extent, with tiles as the unit. */
    TileExtent tileExtent;

    /** The chunks that make up this tile map. */
    std::unordered_map<ChunkPosition, Chunk> chunks;

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

} // End namespace AM
