#pragma once

#include "MoveStatus.h"
#include "NavRequestQueue.h"
#include "NavAgentProfile.h"
#include "NavWaypoint.h"
#include "Vector3.h"
#include "entt/entity/entity.hpp"
#include <SDL3/SDL_stdinc.h>
#include <vector>
#include <cstddef>

namespace AM
{

/**
 * Tracks an entity's movement towards a target. Added and managed by
 * Pathfinder.
 *
 * Note: This is server-side state. It isn't replicated or persisted.
 */
struct PathFollow {
    MoveStatus status{MoveStatus::Moving};

    /** If non-null, the entity is following this entity. */
    entt::entity targetEntity{entt::null};

    /** If targetEntity is null, the position that the entity is moving to. */
    Vector3 targetPosition{};

    /** How close the entity must get to its target, horizontally. */
    float acceptanceRadius{0};

    /** The entity's size and movement abilities. */
    NavAgentProfile agent{};

    /** The current path request, if one is in progress. */
    NavRequestHandle request{};

    /** The path that's being followed. */
    std::vector<NavWaypoint> waypoints{};

    /** The index in waypoints of the next waypoint to move to. */
    std::size_t waypointIndex{0};

    /** The target position that the current path (or request) leads to. */
    Vector3 pathGoal{};

    /** The nav grid version that the current path was found in. */
    Uint32 pathGridVersion{0};

    /** The number of ticks since the last path request. */
    Uint32 ticksSinceRequest{SDL_MAX_UINT32};

    /** Where the entity was at the last progress check (see
        SharedConfig::PATHFINDING_STUCK_TICKS). */
    Vector3 progressCheckPosition{};

    /** The number of ticks since the last progress check. */
    Uint32 ticksSinceProgressCheck{0};

    /** The number of progress checks in a row that the entity has failed. */
    Uint8 failedProgressChecks{0};

    /** If true, the entity will jump on its next grounded tick (to get
        unstuck). */
    bool shouldJump{false};

    /** If true, a new path will be requested (to get unstuck, or because the
        target changed). */
    bool shouldRepath{false};
};

} // End namespace AM
