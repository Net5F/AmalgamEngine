#pragma once

#include "NavSearch.h"
#include "NavAgentProfile.h"
#include "NavWaypoint.h"
#include "TilePosition.h"
#include "SharedConfig.h"
#include <SDL3/SDL_stdinc.h>
#include <vector>
#include <span>
#include <cstddef>

namespace AM
{
class NavGrid;
class NavRegions;

/**
 * A handle to a path request. See NavRequestQueue.
 */
struct NavRequestHandle {
    /** The index of the request's slot. */
    Uint32 index{SDL_MAX_UINT32};

    /** The slot's generation when the request was made. If the slot has
        since been released and reused, this won't match. */
    Uint32 generation{0};

    bool operator==(const NavRequestHandle& other) const = default;
};

/**
 * Queues path requests and searches for them, within a per-tick budget.
 *
 * Usage:
 *   1. Call request() to queue a path request. It returns a handle.
 *   2. Each tick, processRequests() searches for paths, in the order that
 *      they were requested, until the tick's node budget is used up.
 *      Searches that don't finish continue on the next tick.
 *   3. Check getStatus() until it's no longer Idle (queued) or InProgress.
 *      If it's Found, get the path from getWaypoints().
 *   4. Call release() when you're done with the request (or to cancel it).
 *      Every request must be released, so that its slot can be reused.
 *
 * If the nav grid changes while a search is in progress, the search
 * restarts (see NavSearch::run()).
 *
 * Request slots and their waypoint vectors are reused, so this doesn't
 * allocate once it has grown to fit the number of requests in use.
 */
class NavRequestQueue
{
public:
    /**
     * @param inNavRegions (Optional) See NavSearch().
     */
    NavRequestQueue(const NavGrid& inNavGrid,
                    NavRegions* inNavRegions = nullptr);

    /**
     * Queues a path request.
     *
     * See NavSearch::begin() for the parameters.
     *
     * @return A handle to the request. Must be released (see release()).
     */
    NavRequestHandle request(const TilePosition& start,
                             const TilePosition& goal,
                             const NavAgentProfile& agent = {},
                             std::size_t maxExpandedNodes
                             = SharedConfig::PATHFINDING_MAX_SEARCH_NODES);

    /**
     * Releases the given request, cancelling it if it hasn't finished.
     * The handle becomes invalid.
     */
    void release(const NavRequestHandle& handle);

    /**
     * Returns true if the given handle refers to a request that hasn't been
     * released.
     */
    bool isValid(const NavRequestHandle& handle) const;

    /**
     * Returns the given request's status. Idle means it's queued, and its
     * search hasn't started yet.
     *
     * Note: The handle must be valid.
     */
    NavSearch::Status getStatus(const NavRequestHandle& handle) const;

    /**
     * Returns the waypoints of the given request's path. See
     * NavSearch::getWaypoints().
     *
     * Note: The handle must be valid, and the request's status must be
     *       Found.
     */
    std::span<const NavWaypoint>
        getWaypoints(const NavRequestHandle& handle) const;

    /**
     * Searches for queued paths, until the given number of nodes have been
     * expanded or the queue is empty.
     */
    void processRequests(std::size_t nodeBudget
                         = SharedConfig::PATHFINDING_NODES_PER_TICK);

    /**
     * Returns the number of requests that are waiting to be searched,
     * including the one that's currently being searched.
     */
    std::size_t getPendingCount() const;

private:
    /** A path request. */
    struct Request {
        TilePosition start{};
        TilePosition goal{};
        NavAgentProfile agent{};
        std::size_t maxExpandedNodes{0};

        NavSearch::Status status{NavSearch::Status::Idle};

        /** The path that was found, if status is Found. */
        std::vector<NavWaypoint> waypoints{};

        /** Incremented whenever this slot is released, to invalidate old
            handles. */
        Uint32 generation{0};

        /** If true, this slot holds a request that hasn't been released. */
        bool isInUse{false};
    };

    /**
     * If the active request's search finished, saves its result and clears
     * the active request.
     */
    void finishActiveRequest();

    /**
     * Pops queued requests until it finds one that's still valid, and starts
     * its search.
     *
     * @return true if a search was started, else false (the queue is empty).
     */
    bool startNextRequest();

    /** Used to search for paths. Only one search runs at a time. */
    NavSearch search;

    /** The request slots. */
    std::vector<Request> requests;

    /** The indices of request slots that aren't in use. */
    std::vector<Uint32> freeIndices;

    /** The requests that are waiting to be searched, in the order that they
        were requested. Entries before queueFront have already been popped. */
    std::vector<NavRequestHandle> queue;
    std::size_t queueFront;

    /** The request that's currently being searched, if any. */
    NavRequestHandle activeRequest;
};

} // End namespace AM
