#pragma once

#include "NavAgentProfile.h"
#include "NavSurface.h"
#include "SharedConfig.h"
#include <SDL3/SDL_stdinc.h>
#include <vector>
#include <span>
#include <utility>

namespace AM
{
class NavGrid;

/**
 * Tracks which surfaces in a NavGrid can reach each other, so that searches
 * between disconnected areas can fail immediately instead of searching until
 * they run out of nodes.
 *
 * Regions are built for the common agent profile (1 tile wide, with the
 * PATHFINDING_COMMON_* constants in SharedConfig), using the same step rules
 * as NavSearch.
 *
 * Reachability is directional (e.g. an agent may be able to drop off of a
 * ledge, but not climb back up), so regions form a directed graph:
 *   - Within each chunk, surfaces that can reach each other in both
 *     directions are grouped into a "local region".
 *   - Each local region has links to the local regions that it can step to
 *     (within its chunk, or in a neighboring chunk).
 *   - To check if a goal is reachable, we search through the links. There
 *     are far fewer regions than surfaces, so this is much cheaper than a
 *     path search.
 *
 * When chunks are rebuilt, only they and their neighbors' links need to be
 * rebuilt.
 */
class NavRegions
{
public:
    NavRegions(const NavGrid& inNavGrid);

    /**
     * Rebuilds all regions. Call this after NavGrid::build().
     */
    void build();

    /**
     * Rebuilds the regions in the chunks that the nav grid rebuilt in its
     * last update. Call this after NavGrid::update().
     */
    void update();

    /**
     * Returns true if regions can be used to rule out paths for the given
     * agent. This is true if the agent can't go anywhere that the common
     * agent profile can't: it's 1 tile wide, at least as tall, and can't
     * climb or drop any further.
     */
    bool supportsAgent(const NavAgentProfile& agent) const;

    /**
     * Returns false if the common agent profile can't reach the goal surface
     * from the start surface. Otherwise, returns true.
     *
     * If either surface doesn't exist, or the common agent can't stand on it,
     * reachability is unknown and this returns true.
     *
     * Heights are in thirds, relative to the bottom of the nav grid.
     */
    bool isReachable(int startX, int startY, int startHeight, int goalX,
                     int goalY, int goalHeight);

private:
    /** Used to mark surfaces that the common agent can't stand on. */
    static constexpr Uint16 NO_REGION{SDL_MAX_UINT16};

    /** A link to a local region that can be stepped to. */
    struct RegionLink {
        Uint32 chunkIndex{0};
        Uint16 region{0};

        auto operator<=>(const RegionLink&) const = default;
    };

    /** The regions within a chunk. */
    struct ChunkRegions {
        /** The local region of each surface in the chunk (in the same order
            as NavGrid stores them), or NO_REGION. */
        std::vector<Uint16> surfaceRegions;

        /** The number of local regions in this chunk. */
        Uint16 regionCount{0};

        /** Region i's links are at links[linkOffsets[i]] up to (but not
            including) links[linkOffsets[i + 1]]. */
        std::vector<Uint32> linkOffsets;
        std::vector<RegionLink> links;
    };

    /**
     * Groups the surfaces in the given chunk into local regions.
     * Note: This invalidates the links that point to this chunk.
     */
    void buildLocalRegions(int chunkIndex);

    /**
     * Rebuilds the given chunk's links.
     * Note: The local regions in this chunk and its neighbors must be built.
     */
    void buildLinks(int chunkIndex);

    /**
     * Recalculates regionBases. Call after any chunk's region count changes.
     */
    void calcRegionBases();

    /**
     * Returns the origin column of the given chunk.
     */
    std::pair<int, int> getChunkOrigin(int chunkIndex) const;

    /**
     * Returns true if the common agent can stand on the given surface.
     */
    bool isStandable(const NavSurface& surface) const;

    /**
     * Returns true if the common agent can step from one surface to the
     * other, in the given direction.
     * Note: This must match NavSearch's step rules for 1-tile agents.
     */
    bool canStep(const NavSurface& fromSurface, const NavSurface& toSurface,
                 int xOffset, int yOffset) const;

    /** The grid that we build regions for. */
    const NavGrid& navGrid;

    /** The regions within each chunk, indexed the same as the grid's chunks. */
    std::vector<ChunkRegions> chunks;

    /** For each chunk, the index of its first region in the whole grid. Used
        to index visitedMarks. */
    std::vector<Uint32> regionBases;

    /** Scratch vectors for building regions. */
    std::vector<std::span<const NavSurface>> columnSurfaces;
    std::vector<Uint16> columnStarts;
    std::vector<Uint8> surfaceColumns;
    std::vector<Uint16> fillStack;
    std::vector<std::pair<Uint16, RegionLink>> linkScratch;
    std::vector<int> linkChunkIndices;

    /** Scratch data for searching through regions. The visit mark of each
        region is its search ID if it's been visited in the current search. */
    std::vector<Uint32> visitedMarks;
    Uint32 searchID;
    std::vector<RegionLink> frontier;
};

} // End namespace AM
