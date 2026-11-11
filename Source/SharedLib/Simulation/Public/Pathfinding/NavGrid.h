#pragma once

#include "NavNode.h"
#include "TileExtent.h"
#include "BoundingBox.h"
#include "CollisionLayerType.h"
#include "SharedConfig.h"
#include <vector>
#include <utility>

namespace AM
{
class CollisionLocator;
struct TilePosition;

/**
 * A grid of navigation nodes, one per tile, used for pathfinding.
 * See NavNode.h for the data that each node holds.
 *
 * Nodes are derived from the collision volumes in CollisionLocator, so they
 * match what entities actually collide with while moving:
 *   - Terrain provides the walkable surfaces. Other volumes never act as
 *     surfaces (e.g. agents won't path across the tops of objects).
 *   - Volumes that fit within a band along a tile's west or north edge (e.g.
 *     walls) block that edge, if they cover most of it.
 *   - Any other volume above a surface limits its headroom. If it leaves less
 *     than a third of headroom, the node isn't standable.
 *   - Moving entities are ignored, since they never block movement (see
 *     EntityMover.h).
 *
 * Each node holds a single surface. If a tile contains multiple surfaces
 * (e.g. floating terrain, with space beneath it), the highest one is used.
 *
 * Nodes are stored in chunk-sized groups, so that nearby nodes are usually
 * close in memory.
 */
class NavGrid
{
public:
    /** The collision layers that affect navigation. */
    static constexpr CollisionLayerBitSet COLLISION_MASK{
        CollisionLayerType::TerrainWall | CollisionLayerType::Object
        | CollisionLayerType::BlockCollision};

    NavGrid(CollisionLocator& inCollisionLocator);

    /**
     * Sets this grid's size to match the given extent, and clears all nodes.
     *
     * Note: The extent's X and Y axes must be aligned to chunk boundaries.
     */
    void setGridSize(const TileExtent& tileExtent);

    /**
     * Rebuilds every node in this grid, using the current state of the
     * collision locator.
     */
    void build();

    /**
     * Returns the node at the given position.
     * If the position is outside of this grid, returns an empty node (not
     * standable).
     */
    NavNode getNode(const TilePosition& tilePosition) const;

    /**
     * Returns this grid's extent, with tiles as the unit.
     */
    const TileExtent& getTileExtent() const;

private:
    /** A collision volume, copied out of the collision locator. */
    struct Volume {
        BoundingBox bounds{};
        CollisionLayerBitSet collisionLayers{};
    };

    /** The height of a "third" of a tile, in world units. */
    static constexpr float THIRD_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT) / 3.f};

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
     * Returns the index in the nodes vector of the node at the given position.
     * Note: The position must be within this grid.
     */
    std::size_t linearizeIndex(int x, int y, int z) const;

    /**
     * Returns the index in tileVolumeIndices of the tile at the given position.
     * Note: The position must be within the current chunk column.
     */
    std::size_t linearizeColumnIndex(int x, int y, int z) const;

    /**
     * Builds all nodes in the column of chunks with the given origin (except
     * for clearance, which must be built after all columns).
     */
    void buildChunkColumn(int originX, int originY);

    /**
     * Adds the given volume's index to every tile in the current chunk column
     * that it may intersect.
     */
    void addVolumeToTiles(Uint32 volumeIndex);

    /**
     * Calculates all of the given node's data, except for clearance.
     * If the node is standable, its clearance is set to 1.
     */
    NavNode calcNode(int x, int y, int z);

    /**
     * Fills nodeVolumeIndices with the volumes that may affect the given node.
     */
    void gatherNodeVolumes(int x, int y, int z);

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

    /**
     * Calculates the clearance of every standable node in the grid.
     */
    void buildClearance();

    /**
     * Returns the clearance of the given standable node.
     * Note: The nodes to the east, south, and southeast must already have
     *       their clearance built.
     */
    Uint8 calcClearance(int x, int y, int z, const NavNode& node) const;

    /** Used to get the collision volumes that make up the world. */
    CollisionLocator& collisionLocator;

    /** This grid's extent, with tiles as the unit. */
    TileExtent gridTileExtent;

    /** The number of chunks along this grid's X and Y axes. */
    int chunkXLength;
    int chunkYLength;

    /** The nodes that make up this grid.
        Nodes are grouped by chunk, and each group is in row-major order. */
    std::vector<NavNode> nodes;

    /** The origin of the chunk column that's currently being built. */
    int columnOriginX;
    int columnOriginY;

    /** Scratch vector for holding the current chunk column's volumes. */
    std::vector<Volume> columnVolumes;

    /** Scratch vectors, holding the indices in columnVolumes of the volumes
        that may intersect each tile in the current chunk column. */
    std::vector<std::vector<Uint32>> tileVolumeIndices;

    /** Scratch vector, holding the indices in columnVolumes of the volumes
        that may affect the current node. */
    std::vector<Uint32> nodeVolumeIndices;

    /** Scratch vectors for holding the covered intervals of a tile's west and
        north edges. */
    std::vector<std::pair<float, float>> westIntervals;
    std::vector<std::pair<float, float>> northIntervals;
};

} // End namespace AM
