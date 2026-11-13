#pragma once

#include "NavSurface.h"
#include "TileExtent.h"
#include "BoundingBox.h"
#include "CollisionLayerType.h"
#include "SharedConfig.h"
#include <vector>
#include <span>
#include <utility>

namespace AM
{
class CollisionLocator;

/**
 * Navigation data for the tile map, used for pathfinding.
 *
 * Each column of tiles (X, Y) holds a list of the walkable surfaces within
 * it, sorted by increasing height. Heights are absolute (relative to the
 * bottom of the grid), so surfaces don't depend on which tile they're in, and
 * agents can move between tile layers like they move anywhere else.
 *
 * Surfaces are derived from the collision volumes in CollisionLocator, so
 * they match what entities actually collide with while moving:
 *   - Terrain provides the walkable surfaces. Other volumes never act as
 *     surfaces (e.g. agents won't path across the tops of objects).
 *   - Volumes that fit within a band along a tile's west or north edge (e.g.
 *     walls) block that edge, if they cover most of it.
 *   - Any other volume above a surface limits its headroom. If it leaves less
 *     than a third of headroom, there's no surface.
 *   - Moving entities are ignored, since they never block movement (see
 *     EntityMover.h).
 *
 * Surfaces are stored per chunk (a 16x16 column of tiles, through all tile
 * layers). Chunks without any surfaces don't allocate anything, so memory
 * scales with the walkable area of the map, not its volume.
 *
 * To keep the surfaces up to date, this grid enables change tracking on the
 * collision locator (for COLLISION_MASK). Call update() each tick to process
 * the changes.
 */
class NavGrid
{
public:
    /** The collision layers that affect navigation. */
    static constexpr CollisionLayerBitSet COLLISION_MASK{
        CollisionLayerType::TerrainWall | CollisionLayerType::Object
        | CollisionLayerType::BlockCollision};

    /** The height of a "third" of a tile, in world units. */
    static constexpr float THIRD_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT) / 3.f};

    NavGrid(CollisionLocator& inCollisionLocator);

    /**
     * Sets this grid's size to match the given extent, and clears all
     * surfaces.
     *
     * Note: The extent's X and Y axes must be aligned to chunk boundaries.
     */
    void setGridSize(const TileExtent& tileExtent);

    /**
     * Rebuilds every surface in this grid, using the current state of the
     * collision locator.
     * Any changes that the locator tracked before this call are discarded.
     */
    void build();

    /**
     * Rebuilds the chunks affected by any changes that the collision locator
     * has tracked since the last update or build.
     */
    void update();

    /**
     * Returns the surfaces in the given column, sorted by increasing height.
     * If the column is outside of this grid, returns an empty span.
     *
     * Note: The span is invalidated when the column's chunk is rebuilt.
     */
    std::span<const NavSurface> getSurfaces(int x, int y) const;

    /**
     * Returns the surface at the given height in the given column, or nullptr
     * if there isn't one.
     */
    const NavSurface* findSurface(int x, int y, int height) const;

    /**
     * Returns this grid's extent, with tiles as the unit.
     */
    const TileExtent& getTileExtent() const;

    /**
     * Returns this grid's version, which is incremented whenever any surfaces
     * change.
     */
    Uint32 getVersion() const;

    /**
     * Returns the world Z coordinate of the given height (in thirds, relative
     * to the bottom of this grid).
     */
    float getWorldZ(int height) const;

    /**
     * Returns the number of chunks in this grid.
     * Chunks are indexed in row-major order, starting at the grid's origin.
     */
    int getChunkCount() const;

    /**
     * Returns the index of the chunk containing the given column.
     * Note: The column must be within this grid.
     */
    int getChunkIndex(int x, int y) const;

    /**
     * Returns the index of the given surface within its chunk.
     * Surfaces are indexed in the same order as they're stored: by column (in
     * row-major order), then by increasing height.
     *
     * Note: The surface must be from getSurfaces() or findSurface(), and
     *       chunkIndex must be the index of the chunk that contains it.
     */
    std::size_t getSurfaceIndex(int chunkIndex,
                                const NavSurface& surface) const;

    /**
     * Returns the indices of the chunks that were rebuilt by the last call to
     * update().
     */
    const std::vector<int>& getUpdatedChunks() const;

private:
    /** A collision volume, copied out of the collision locator. */
    struct Volume {
        BoundingBox bounds{};
        CollisionLayerBitSet collisionLayers{};
    };

    /**
     * The surfaces within a chunk.
     *
     * Note: If we ever need to save memory on large maps, chunks with exactly
     *       one surface in every column (common on flat ground) could drop
     *       their column offsets, since column i's surface would just be
     *       surfaces[i]. That would bring flat ground from roughly 6 to 4
     *       bytes per column.
     * TODO: If chunk lookups show up when profiling searches, we could store
     *       columnOffsets and surfaces in a single buffer per chunk. Getting
     *       a column's surfaces currently goes through 3 separate
     *       allocations (chunks, columnOffsets, surfaces), so a search
     *       entering a new chunk may take 3 cache misses. A single buffer
     *       would reduce that, and halve the allocations when rebuilding.
     */
    struct Chunk {
        /** The index in surfaces where each column's surfaces start.
            Column i's surfaces start at columnOffsets[i], and end just
            before columnOffsets[i + 1] (where the next column's start).
            This holds one extra element at the end (CHUNK_TILE_COUNT + 1
            total), so the last column also has an end.
            Empty if this chunk has no surfaces.
            Note: Since these are Uint16, a chunk can hold at most
                  SDL_MAX_UINT16 surfaces (see buildChunk()). */
        std::vector<Uint16> columnOffsets;

        /** The surfaces in this chunk, grouped by column (in row-major order)
            and sorted by increasing height within each column. */
        std::vector<NavSurface> surfaces;
    };

    /** The width of the band along a tile's west and north edges. Volumes
        that fit within it are treated as edges (e.g. walls) instead of as
        obstacles. */
    static constexpr float EDGE_BAND_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH) / 4.f};

    /** If edge volumes cover more than this length of an edge, the edge is
        blocked. */
    static constexpr float EDGE_BLOCKED_LENGTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH) / 2.f};

    /**
     * Rebuilds all of the surfaces in the given chunk.
     */
    void buildChunk(int chunkIndex);

    /**
     * Adds the given volume's index to the volume lists of every column in the
     * current chunk that it may intersect.
     */
    void addVolumeToColumns(Uint32 volumeIndex, int chunkOriginX,
                            int chunkOriginY);

    /**
     * Calculates the given column's surfaces and adds them to
     * chunkSurfaces.
     */
    void buildColumn(int x, int y, const std::vector<Uint32>& volumeIndices);

    /**
     * Calculates the surface at the given height in the given column.
     *
     * @return true if there's room for a surface, else false.
     */
    bool calcSurface(int x, int y, int height,
                     const std::vector<Uint32>& volumeIndices,
                     NavSurface& surface);

    /**
     * Returns true if the given bounds cover the entire X/Y area of the given
     * tile.
     */
    bool coversTile(const BoundingBox& bounds, float tileMinX,
                    float tileMinY) const;

    /**
     * Returns the total length covered by the given intervals.
     * Note: The given vector will be sorted.
     */
    float calcCoveredLength(std::vector<std::pair<float, float>>& intervals);

    /** Used to get the collision volumes that make up the world. */
    CollisionLocator& collisionLocator;

    /** This grid's extent, with tiles as the unit. */
    TileExtent gridTileExtent;

    /** The number of chunks along this grid's X and Y axes. */
    int chunkXLength;
    int chunkYLength;

    /** The chunks that make up this grid, in row-major order. */
    std::vector<Chunk> chunks;

    /** This grid's version. See getVersion(). */
    Uint32 version;

    /** Scratch vector for holding the current chunk's volumes. */
    std::vector<Volume> chunkVolumes;

    /** Scratch vectors, holding the indices in chunkVolumes of the volumes
        that may intersect each column in the current chunk. */
    std::vector<std::vector<Uint32>> columnVolumeIndices;

    /** Scratch vectors for building the current chunk's surfaces. */
    std::vector<Uint16> chunkColumnOffsets;
    std::vector<NavSurface> chunkSurfaces;

    /** Scratch vector, holding the candidate surface heights in the current
        column. */
    std::vector<int> candidateHeights;

    /** Scratch vectors for holding the covered intervals of a tile's west and
        north edges. */
    std::vector<std::pair<float, float>> westIntervals;
    std::vector<std::pair<float, float>> northIntervals;

    /** Scratch vector, holding the indices of chunks that need rebuilding. */
    std::vector<int> dirtyChunkIndices;
};

} // End namespace AM
