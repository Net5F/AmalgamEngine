#include "catch2/catch_all.hpp"
#include "NavTestMap.h"
#include "NavRequestQueue.h"

using namespace AM;

namespace
{
/**
 * A test map with a request queue.
 */
struct QueueTestMap : public NavTestMap {
    NavRequestQueue queue{navGrid};

    QueueTestMap()
    {
        fillTerrain(0, Terrain::Height::Flat);
        build();
    }

    /** Processes requests until the given request finishes.
        @return The number of ticks that it took. */
    int processUntilFinished(const NavRequestHandle& handle,
                             std::size_t nodeBudget)
    {
        int tickCount{0};
        do {
            queue.processRequests(nodeBudget);
            tickCount++;
        } while (isPending(queue.getStatus(handle)) && (tickCount < 1000));
        return tickCount;
    }

    bool isPending(NavSearch::Status status)
    {
        return (status == NavSearch::Status::Idle)
               || (status == NavSearch::Status::InProgress);
    }
};
} // namespace

TEST_CASE_METHOD(QueueTestMap, "NavRequestQueue single request")
{
    NavRequestHandle handle{queue.request({0, 0, 0}, {5, 3, 0})};
    REQUIRE(queue.isValid(handle));
    REQUIRE(queue.getStatus(handle) == NavSearch::Status::Idle);
    REQUIRE(queue.getPendingCount() == 1);

    // A short path finishes within one tick.
    queue.processRequests();
    REQUIRE(queue.getStatus(handle) == NavSearch::Status::Found);
    REQUIRE(queue.getWaypoints(handle).back().tilePosition
            == TilePosition{5, 3, 0});
    REQUIRE(queue.getPendingCount() == 0);

    // After it's released, the handle is invalid.
    queue.release(handle);
    REQUIRE(!(queue.isValid(handle)));
}

TEST_CASE_METHOD(QueueTestMap, "NavRequestQueue spreads searches across ticks")
{
    NavRequestHandle handle{queue.request({-14, -14, 0}, {14, 14, 0})};

    // With a small budget, the search takes multiple ticks.
    queue.processRequests(5);
    REQUIRE(queue.getStatus(handle) == NavSearch::Status::InProgress);
    int tickCount{1 + processUntilFinished(handle, 5)};
    REQUIRE(tickCount > 2);
    REQUIRE(queue.getStatus(handle) == NavSearch::Status::Found);
    REQUIRE(queue.getWaypoints(handle).back().tilePosition
            == TilePosition{14, 14, 0});
}

TEST_CASE_METHOD(QueueTestMap, "NavRequestQueue processes requests in order")
{
    NavRequestHandle first{queue.request({-14, -14, 0}, {14, 14, 0})};
    NavRequestHandle second{queue.request({0, 0, 0}, {1, 0, 0})};
    REQUIRE(queue.getPendingCount() == 2);

    // The second request waits until the first finishes.
    queue.processRequests(5);
    REQUIRE(queue.getStatus(first) == NavSearch::Status::InProgress);
    REQUIRE(queue.getStatus(second) == NavSearch::Status::Idle);

    processUntilFinished(first, 5);
    REQUIRE(queue.getStatus(first) == NavSearch::Status::Found);

    processUntilFinished(second, 5);
    REQUIRE(queue.getStatus(second) == NavSearch::Status::Found);

    // Multiple short requests can finish in a single tick.
    NavRequestHandle third{queue.request({0, 0, 0}, {2, 0, 0})};
    NavRequestHandle fourth{queue.request({0, 0, 0}, {0, 2, 0})};
    queue.processRequests();
    REQUIRE(queue.getStatus(third) == NavSearch::Status::Found);
    REQUIRE(queue.getStatus(fourth) == NavSearch::Status::Found);
}

TEST_CASE_METHOD(QueueTestMap, "NavRequestQueue failed requests")
{
    // Requests that fail before searching don't use any of the budget.
    // Note: The valid request expands its start, then finds the goal.
    NavRequestHandle invalidStart{queue.request({0, 0, 2}, {5, 0, 0})};
    NavRequestHandle unreachable{queue.request({0, 0, 0}, {100, 0, 0})};
    NavRequestHandle valid{queue.request({0, 0, 0}, {1, 0, 0})};
    queue.processRequests(2);
    REQUIRE(queue.getStatus(invalidStart) == NavSearch::Status::InvalidStart);
    REQUIRE(queue.getStatus(unreachable) == NavSearch::Status::Unreachable);
    REQUIRE(queue.getStatus(valid) == NavSearch::Status::Found);

    // Searches that hit their node limit report it.
    NavRequestHandle limited{queue.request({-14, -14, 0}, {14, 14, 0}, {}, 3)};
    processUntilFinished(limited, 100);
    REQUIRE(queue.getStatus(limited) == NavSearch::Status::NodeLimitReached);
}

TEST_CASE_METHOD(QueueTestMap, "NavRequestQueue releasing requests")
{
    NavRequestHandle first{queue.request({-14, -14, 0}, {14, 14, 0})};
    NavRequestHandle second{queue.request({-14, 14, 0}, {14, -14, 0})};
    NavRequestHandle third{queue.request({0, 0, 0}, {1, 0, 0})};

    // Release the first while it's being searched, and the second while it's
    // queued. The third should then be searched.
    queue.processRequests(5);
    REQUIRE(queue.getStatus(first) == NavSearch::Status::InProgress);
    queue.release(first);
    queue.release(second);
    REQUIRE(queue.getPendingCount() == 1);
    queue.processRequests(5);
    REQUIRE(queue.getStatus(third) == NavSearch::Status::Found);

    // Released slots are reused, but old handles stay invalid.
    NavRequestHandle reused{queue.request({0, 0, 0}, {2, 0, 0})};
    REQUIRE(((reused.index == first.index) || (reused.index == second.index)));
    REQUIRE(queue.isValid(reused));
    REQUIRE(!(queue.isValid(first)));
    REQUIRE(!(queue.isValid(second)));
}

TEST_CASE_METHOD(QueueTestMap, "NavRequestQueue grid changes")
{
    // A wall across the map, with a gap at y == 0.
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        if (y != 0) {
            addWall({1, y, 0}, Wall::Type::West);
        }
    }
    update();

    // Start a search, then close the gap while it's in progress. The search
    // should restart and see the change.
    NavRequestHandle handle{queue.request({-5, -10, 0}, {5, -10, 0})};
    queue.processRequests(5);
    REQUIRE(queue.getStatus(handle) == NavSearch::Status::InProgress);

    addWall({1, 0, 0}, Wall::Type::West);
    update();
    processUntilFinished(handle, 5);
    REQUIRE(queue.getStatus(handle) == NavSearch::Status::Unreachable);
}
