#pragma once

#include "NavAgentProfile.h"
#include "NavWaypoint.h"
#include "NavSurface.h"
#include "TilePosition.h"
#include "SharedConfig.h"
#include <SDL3/SDL_stdinc.h>
#include <vector>
#include <optional>
#include <cstddef>

namespace AM
{
class NavGrid;

/**
 * Finds paths through a NavGrid, using an 8-direction A* search.
 *
 * An agent's footprint is a square of columns, anchored at its northwest
 * column. The agent rests on the highest surface under its footprint, and
 * may hang over lower surfaces. Each search node is an anchor column plus the
 * agent's resting height. Heights are measured in thirds (see NavSurface.h).
 *
 * Agents move between neighboring anchors, following these step rules:
 *   - Every column in the footprint must have a surface at or below the
 *     agent, with enough headroom above it for the agent. At least one of
 *     those surfaces must be at the agent's resting height.
 *   - There must be no blocked edges between the columns in the footprint.
 *   - Cardinal steps may climb up to the agent's climb limit (by jumping), or
 *     drop up to its drop limit. While stepping, the agent is at the higher
 *     of the two heights, so both footprints need enough headroom for it.
 *   - Diagonal steps must be flat. The two cardinal neighbors that the step
 *     passes between must also fit the agent at the same height, and there
 *     must be no blocked edges between them (no corner cutting).
 *   - Blocked edges can't be crossed. For agents wider than 1 tile, the
 *     crossed edges are inside the destination footprint, so they're covered
 *     by the footprint check.
 *
 * Start and goal positions are given as tiles. The agent is placed at the
 * highest valid resting height within the tile's layer.
 *
 * A search can be spread across multiple calls: begin() it, then call run()
 * until it returns something other than InProgress. If the nav grid changes
 * between calls, the search restarts.
 */
class NavSearch
{
public:
    enum class Status : Uint8 {
        /** No search has been started. */
        Idle,
        /** The search hasn't finished yet. Call run() to continue it. */
        InProgress,
        /** A path was found. See getWaypoints(). */
        Found,
        /** The agent doesn't fit at the start (e.g. there's no surface, or
            there isn't enough headroom). */
        InvalidStart,
        /** There's no path from the start to the goal. */
        Unreachable,
        /** The search expanded its max number of nodes without reaching the
            goal. */
        NodeLimitReached
    };

    NavSearch(const NavGrid& inNavGrid);

    /**
     * Runs a complete search. See begin() for the parameters.
     *
     * @return The final status of the search.
     */
    Status findPath(const TilePosition& start, const TilePosition& goal,
                    const NavAgentProfile& inAgent = {},
                    std::size_t inMaxExpandedNodes
                    = SharedConfig::PATHFINDING_MAX_SEARCH_NODES);

    /**
     * Starts a new search. Any search in progress is discarded.
     *
     * @param start The tile that the agent's footprint is anchored at.
     * @param goal The tile to anchor the agent's footprint at.
     * @param inAgent The agent to search for.
     * @param inMaxExpandedNodes The max number of nodes to expand before
     *                           giving up.
     */
    void begin(const TilePosition& start, const TilePosition& goal,
               const NavAgentProfile& inAgent = {},
               std::size_t inMaxExpandedNodes
               = SharedConfig::PATHFINDING_MAX_SEARCH_NODES);

    /**
     * Continues the current search, expanding up to the given number of
     * nodes.
     *
     * If the nav grid changed since the last call, the search restarts.
     *
     * @return The search's status.
     */
    Status run(std::size_t maxExpansions);

    Status getStatus() const;

    /**
     * Returns the waypoints of the path that was found, not including the
     * start. Collinear waypoints along flat ground are skipped.
     * If the start is the goal, this will be empty.
     *
     * Note: Only valid if the search's status is Found.
     */
    const std::vector<NavWaypoint>& getWaypoints() const;

    /**
     * Returns the number of nodes that the current search has expanded.
     */
    std::size_t getExpandedNodeCount() const;

    /**
     * Returns the number of nodes that the last call to run() expanded.
     */
    std::size_t getLastRunExpansionCount() const;

private:
    /** Used to mark records that have no parent. */
    static constexpr Uint32 NO_PARENT{SDL_MAX_UINT32};

    /** Used to mark nodes that have no record in the current search. */
    static constexpr Uint32 NO_RECORD{SDL_MAX_UINT32};

    /** log2 of the initial number of slots in the record lookup table. */
    static constexpr int INITIAL_SLOT_BITS{10};

    /** Search data for a node that has been reached. */
    struct NodeRecord {
        /** The column that the agent's footprint is anchored at. */
        int x{0};
        int y{0};
        /** The agent's resting height, in thirds. */
        int height{0};
        /** The cost of the best known path from the start to this node. */
        float gCost{0};
        /** The index in records of the previous node along the path. */
        Uint32 parentIndex{NO_PARENT};
        /** If true, reaching this node from its parent requires a jump. */
        bool isJump{false};
        /** If true, this node has been expanded. */
        bool isClosed{false};
    };

    /** A slot in the record lookup table. */
    struct RecordSlot {
        /** The node's key. See calcKey(). */
        Uint64 key{0};
        /** The index in records of the node's record. */
        Uint32 recordIndex{NO_RECORD};
        /** The search that this slot was filled in. If it doesn't match the
            current search, this slot is empty. */
        Uint32 searchID{0};
    };

    /** An entry in the open list. */
    struct OpenEntry {
        float fCost{0};
        float hCost{0};
        Uint32 recordIndex{0};
    };

    /** Orders the open list so that the lowest f cost is on top. Ties go to
        the entry that's closest to the goal. */
    struct IsLowerPriority {
        bool operator()(const OpenEntry& a, const OpenEntry& b) const
        {
            return (a.fCost > b.fCost)
                   || ((a.fCost == b.fCost) && (a.hCost > b.hCost));
        }
    };

    /**
     * Expands the neighbors of the given record's node.
     */
    void expandNode(Uint32 recordIndex);

    /**
     * Adds any valid cardinal steps from the given node in the given
     * direction.
     */
    void expandCardinal(Uint32 recordIndex, int fromX, int fromY,
                        int fromHeight, int xOffset, int yOffset);

    /**
     * Adds the diagonal step from the given node in the given direction, if
     * it's valid.
     */
    void expandDiagonal(Uint32 recordIndex, int fromX, int fromY, int height,
                        int xOffset, int yOffset);

    /**
     * Adds the given node to the open list, if this is the best known path to
     * it.
     */
    void relaxNode(int x, int y, int height, float gCost, Uint32 parentIndex,
                   bool isJump);

    /**
     * Returns true if the agent's footprint fits when anchored at the given
     * column, resting at the given height.
     *
     * @param liftedHeight The height that the agent's feet are actually at.
     *                     Normally equal to restingHeight, but may be higher
     *                     while the agent is stepping up or down.
     */
    bool fitsFootprint(int anchorX, int anchorY, int restingHeight,
                       int liftedHeight) const;

    /**
     * Returns the highest surface in the given column that's at or below the
     * given height, or nullptr if there isn't one.
     */
    const NavSurface* findSurfaceBelow(int x, int y, int height) const;

    /**
     * Returns true if a small agent at the given surfaces can't cross the
     * edge between them.
     */
    bool isEdgeBlocked(int fromX, int fromY, const NavSurface& fromSurface,
                       int toX, int toY, const NavSurface& toSurface) const;

    /**
     * Returns the highest height within the given tile's layer that the agent
     * can rest at, when anchored at the tile's column.
     */
    std::optional<int> findRestingHeight(const TilePosition& tilePosition);

    /**
     * Fills candidateHeights with the heights of the surfaces within the
     * given footprint that are in the given range, sorted from highest to
     * lowest.
     */
    void findCandidateHeights(int anchorX, int anchorY, int minHeight,
                              int maxHeight);

    /**
     * Returns the estimated cost from the given node to the goal.
     */
    float calcHeuristic(int x, int y, int height) const;

    /**
     * Returns a unique key for the given node.
     */
    Uint64 calcKey(int x, int y, int height) const;

    /**
     * Returns a reference to the given node's record index (NO_RECORD if it
     * doesn't have a record yet).
     *
     * Note: The reference is invalidated by the next call.
     */
    Uint32& getRecordIndex(int x, int y, int height);

    /**
     * Returns the slot that holds the given key in the current search, or
     * the empty slot where it should be added.
     */
    RecordSlot& findSlot(Uint64 key);

    /**
     * Doubles the size of the record lookup table, and re-adds the current
     * search's records to it.
     */
    void growRecordSlots();

    /**
     * Fills waypoints with the path that ends at the given record.
     */
    void buildWaypoints(Uint32 goalRecordIndex);

    /**
     * Returns a / b, rounded towards negative infinity.
     */
    int floorDivide(int a, int b) const;

    /** The grid to search through. */
    const NavGrid& navGrid;

    /** The current search's parameters. */
    TilePosition startPosition;
    TilePosition goalPosition;
    NavAgentProfile agent;
    std::size_t maxExpandedNodes;

    /** The goal node's resting height. */
    int goalHeight;

    /** The nav grid version that the current search started with. */
    Uint32 gridVersion;

    /** The current search's status. */
    Status status;

    /** The number of nodes that the current search has expanded. */
    std::size_t expandedNodeCount;

    /** The number of nodes that the last call to run() expanded. */
    std::size_t lastRunExpansionCount;

    /** The nodes that the current search has reached. */
    std::vector<NodeRecord> records;

    // Note: To find a node's record without allocating during searches, we
    //       use an open-addressing hash table that's reused across searches.
    //       It only grows until it fits the largest search we've done.
    /** The record lookup table. Its size is always a power of 2. */
    std::vector<RecordSlot> recordSlots;

    /** log2 of the size of recordSlots. */
    int recordSlotBits;

    /** The current search's ID. Incremented for each search, so that old
        record slots are invalidated without needing to clear them. */
    Uint32 searchID;

    /** The open list, as a min-heap ordered by f cost. */
    std::vector<OpenEntry> openList;

    /** The waypoints of the path that was found. */
    std::vector<NavWaypoint> waypoints;

    /** Scratch vector, holding the record indices along the found path. */
    std::vector<Uint32> pathRecordIndices;

    /** Scratch vector, holding the candidate resting heights for a step. */
    std::vector<int> candidateHeights;
};

} // End namespace AM
