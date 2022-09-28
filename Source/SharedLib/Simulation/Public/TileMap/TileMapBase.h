#pragma once

#include "Tile.h"
#include "ChunkExtent.h"
#include "TileExtent.h"
#include <vector>
#include <unordered_set>

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
     * Sets the given sprite layer to the given tile.
     *
     * If the given tile's spriteLayers vector isn't big enough, resizes
     * it. Any tiles added during resizing will be default initialized to
     * the "empty sprite".
     *
     * Note: There's no bounds checking on tileX/tileY. It's on you to make
     *       sure they're valid.
     */
    void setTileSpriteLayer(int tileX, int tileY,
                            unsigned int layerIndex, const Sprite& sprite);

    /**
     * Overload for sprite string IDs.
     */
    void setTileSpriteLayer(int tileX, int tileY,
                            unsigned int layerIndex,
                            const std::string& stringID);

    /**
     * Overload for sprite numeric IDs.
     */
    void setTileSpriteLayer(int tileX, int tileY,
                            unsigned int layerIndex, int numericID);

    /**
     * Clears all sprite layers out of the given tile.
     *
     * Note: There's no bounds checking on tileX/tileY. It's on you to make
     *       sure they're valid.
     *
     * @return true if any layers were cleared. false if the tile was empty.
     */
    bool clearTile(int tileX, int tileY);

    /**
     * Overload for clearing a chosen layer and all layers above it.
     *
     * @param startLayerIndex  The layer index to start clearing at.
     *                         E.g. If 1, layers 1 - end will be cleared and 
     *                         layer 0 will remain untouched.
     * @return true if any layers were cleared. false if startLayerIndex was 
     *         too high to match any layers in the given tile.
     */
    bool clearTile(int tileX, int tileY,
                   unsigned int startLayerIndex);

    /**
     * Clears sprite layers out of all tiles within the given extent.
     *
     * Note: There's no bounds checking on the given extent. It's on you to make
     *       sure the indices are valid.
     *
     * @return true if any layers were cleared. false if all tiles were empty.
     */
    bool clearExtent(TileExtent extent);

    /**
     * Overload for clearing a chosen layer and all layers above it.
     *
     * @param startLayerIndex  The layer index to start clearing at.
     *                         E.g. If 1, layers 1 - end will be cleared and 
     *                         layer 0 will remain untouched.
     * @return true if any layers were cleared. false if startLayerIndex was 
     *         too high to match any layers in the given tile.
     */
    bool clearExtent(TileExtent extent, unsigned int startLayerIndex);

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
     * Returns the coordinates of all tiles that have been modified.
     *
     * Note: This class does not clear elements from this container. You 
     *       must do it yourself after you've processed them.
     */
    std::unordered_set<TilePosition>& getDirtyTiles();

protected:
    /**
     * Returns the index in the tiles vector where the tile with the given
     * coordinates can be found.
     */
    inline unsigned int linearizeTileIndex(int x, int y) const
    {
        return (y * tileExtent.xLength) + x;
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

    /** Holds the coordinates of tiles that currently have dirty state. */
    std::unordered_set<TilePosition> dirtyTiles;
};

} // End namespace AM
