#include "NavRequestQueue.h"
#include "AMAssert.h"
#include "tracy/Tracy.hpp"

namespace AM
{

NavRequestQueue::NavRequestQueue(const NavGrid& inNavGrid)
: search{inNavGrid}
, requests{}
, freeIndices{}
, queue{}
, queueFront{0}
, activeRequest{}
{
}

NavRequestHandle NavRequestQueue::request(const TilePosition& start,
                                          const TilePosition& goal,
                                          const NavAgentProfile& agent,
                                          std::size_t maxExpandedNodes)
{
    // Get a free slot.
    Uint32 index{};
    if (!(freeIndices.empty())) {
        index = freeIndices.back();
        freeIndices.pop_back();
    }
    else {
        index = static_cast<Uint32>(requests.size());
        requests.emplace_back();
    }

    // Fill it and queue it.
    Request& request{requests[index]};
    request.start = start;
    request.goal = goal;
    request.agent = agent;
    request.maxExpandedNodes = maxExpandedNodes;
    request.status = NavSearch::Status::Idle;
    request.waypoints.clear();
    request.isInUse = true;

    NavRequestHandle handle{index, request.generation};
    queue.push_back(handle);
    return handle;
}

void NavRequestQueue::release(const NavRequestHandle& handle)
{
    AM_ASSERT(isValid(handle), "Tried to release an invalid path request.");
    if (!isValid(handle)) {
        return;
    }

    // If this request is being searched, stop.
    // Note: If it's still queued, its queue entry will be skipped since its
    //       generation won't match.
    if (activeRequest == handle) {
        activeRequest = {};
    }

    Request& request{requests[handle.index]};
    request.isInUse = false;
    request.generation++;
    freeIndices.push_back(handle.index);
}

bool NavRequestQueue::isValid(const NavRequestHandle& handle) const
{
    return (handle.index < requests.size()) && requests[handle.index].isInUse
           && (requests[handle.index].generation == handle.generation);
}

NavSearch::Status
    NavRequestQueue::getStatus(const NavRequestHandle& handle) const
{
    AM_ASSERT(isValid(handle), "Tried to use an invalid path request.");
    return requests[handle.index].status;
}

std::span<const NavWaypoint>
    NavRequestQueue::getWaypoints(const NavRequestHandle& handle) const
{
    AM_ASSERT(isValid(handle), "Tried to use an invalid path request.");
    AM_ASSERT(requests[handle.index].status == NavSearch::Status::Found,
              "Tried to get waypoints from an unsuccessful path request.");
    return requests[handle.index].waypoints;
}

void NavRequestQueue::processRequests(std::size_t nodeBudget)
{
    ZoneScoped;

    std::size_t remainingBudget{nodeBudget};
    while (remainingBudget > 0) {
        // If there's no active request, start the next one.
        if (!isValid(activeRequest) && !startNextRequest()) {
            // The queue is empty.
            break;
        }

        // Continue the search, and use up the nodes that it expanded.
        search.run(remainingBudget);
        remainingBudget -= search.getLastRunExpansionCount();
        finishActiveRequest();
    }

    // Remove the popped entries from the queue, so it doesn't grow forever.
    // Note: We only do this once at least half of the queue has been popped,
    //       to avoid shifting the remaining entries every tick.
    if ((queueFront > 0) && ((queueFront * 2) >= queue.size())) {
        queue.erase(queue.begin(),
                    queue.begin() + static_cast<std::ptrdiff_t>(queueFront));
        queueFront = 0;
    }
}

std::size_t NavRequestQueue::getPendingCount() const
{
    std::size_t pendingCount{0};
    for (std::size_t i{queueFront}; i < queue.size(); ++i) {
        if (isValid(queue[i])) {
            pendingCount++;
        }
    }

    if (isValid(activeRequest)) {
        pendingCount++;
    }

    return pendingCount;
}

void NavRequestQueue::finishActiveRequest()
{
    NavSearch::Status status{search.getStatus()};
    Request& request{requests[activeRequest.index]};
    request.status = status;
    if (status == NavSearch::Status::InProgress) {
        return;
    }

    if (status == NavSearch::Status::Found) {
        const std::vector<NavWaypoint>& waypoints{search.getWaypoints()};
        request.waypoints.assign(waypoints.begin(), waypoints.end());
    }
    activeRequest = {};
}

bool NavRequestQueue::startNextRequest()
{
    while (queueFront < queue.size()) {
        NavRequestHandle handle{queue[queueFront]};
        queueFront++;

        // Skip any requests that were released while queued.
        if (!isValid(handle)) {
            continue;
        }

        const Request& request{requests[handle.index]};
        activeRequest = handle;
        search.begin(request.start, request.goal, request.agent,
                     request.maxExpandedNodes);
        return true;
    }

    return false;
}

} // End namespace AM
