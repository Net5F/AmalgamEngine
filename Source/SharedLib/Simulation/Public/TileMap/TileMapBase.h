#pragma once

#include "Tile.h"
#include "ChunkExtent.h"
#include "TileExtent.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace AM
{
struct TileMapSnapshot;
class SpriteDataBase;

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
     */
    TileMapBase(SpriteDataBase& inSpriteData, bool inTrackDirtyState);

    /**
     * Sets the layer at the given index to the given sprite.
     *
     * If the given tile's spriteLayers vector isn't big enough, resizes
     * it. Any tiles added during resizing will be default initialized to
     * the "empty sprite".
     *
     * Note: There's no bounds checking on tileX/tileY. It's on you to make
     *       sure they're valid.
     */
    void setTileSpriteLayer(int tileX, int tileY, std::size_t layerIndex,
                            const Sprite& sprite);

    /**
     * Overload for sprite string IDs.
     */
    void setTileSpriteLayer(int tileX, int tileY, std::size_t layerIndex,
                            const std::string& stringID);

    /**
     * Overload for sprite numeric IDs.
     */
    void setTileSpriteLayer(int tileX, int tileY, std::size_t layerIndex,
                            int numericID);

    /**
     * Clears all layers between the given indices (inclusive) in the
     * given tile.
     *
     * It's valid to use the same index as the start and end.
     *
     * It's safe to use an end index that's past the end of the tile's vector,
     * it will be constrained.
     *
     * If clearing to the end of the vector, layers will be erased. Otherwise,
     * they'll be set to the "empty sprite".
     *
     * @param startLayerIndex  The layer index to start clearing at.
     * @param endLayerIndex  The last layer index to clear. Must be >= start.
     * @return true if any layers were cleared.
     */
    bool clearTileSpriteLayers(int tileX, int tileY,
                               std::size_t startLayerIndex,
                               std::size_t endLayerIndex);

    /**
     * Clears all sprite layers in the given tile.
     *
     * Note: There's no bounds checking on tileX/tileY. It's on you to make
     *       sure they're valid.
     *
     * @return true if any layers were cleared. false if the tile was empty.
     */
    bool clearTile(int tileX, int tileY);

    /**
     * Clears all layers between the given indices (inclusive) in all tiles
     * within the given extent.
     *
     * See clearTileSpriteLayers() for behavior.
     */
    bool clearExtentSpriteLayers(TileExtent extent, std::size_t startLayerIndex,
                                 std::size_t endLayerIndex);

    /**
     * Clears all sprite layers in all tiles within the given extent.
     *
     * Note: There's no bounds checking on the given extent. It's on you to make
     *       sure the indices are valid.
     *
     * @return true if any layers were cleared. false if all tiles were empty.
     */
    bool clearExtent(TileExtent extent);

    /**
     * Clears all tile map state, leaving an empty map.
     */
    void clear();

    /**
     * Gets a const reference to the tile at the given coordinates.
     */
    const Tile& getTile(int x, int y) const;

    /**
     * Returns the map extent, with chunks as the unit.
     */
    const ChunkExtent& getChunkExtent() const;

    /**
     * Returns the map extent, with tiles as the unit.
     */
    const TileExtent& getTileExtent() const;

    /**
     * Returns a map containing all tiles with dirty state, and the lowest
     * dirty layer index for each tile.
     *
     * Note: This class does not clear elements from this container. You
     *       must do it yourself after you've processed them.
     */
    std::unordered_map<TilePosition, std::size_t>& getDirtyTiles();

protected:
    /**
     * Returns the index in the tiles vector where the tile with the given
     * coordinates can be found.
     */
    inline std::size_t linearizeTileIndex(int x, int y) const
    {
        return static_cast<std::size_t>((y * tileExtent.xLength) + x);
    }

    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    static constexpr uint16_t MAP_FORMAT_VERSION = 0;

    /** Used to get sprites while constructing tiles. */
    SpriteDataBase& spriteData;

    /** The map's extent, with chunks as the unit. */
    ChunkExtent chunkExtent;

    /** The map's extent, with tiles as the unit. */
    TileExtent tileExtent;

    /** The tiles that make up this map, stored in row-major order. */
    std::vector<Tile> tiles;

private:
    /** If true, any updates to a tile's state will cause that tile to be
        pushed into dirtyTiles. */
    bool trackDirtyState;

    /** Tracks the lowest dirty layer for each dirty tile.
        If dirtyTiles[{x, y}] == 2, layer 2 is the lowest dirty layer for tile
        (x, y). */
    std::unordered_map<TilePosition, std::size_t> dirtyTiles;
};

} // End namespace AM
