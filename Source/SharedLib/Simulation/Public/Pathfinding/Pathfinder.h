#pragma once

#include "NavGrid.h"
#include "NavRequestQueue.h"
#include "MoveStatus.h"
#include "PathFollow.h"
#include "Input.h"
#include "SharedConfig.h"
#include "entt/fwd.hpp"
#include <optional>

namespace AM
{
class CollisionLocator;
struct TileExtent;
struct Position;
struct Movement;
struct MovementModifiers;

/**
 * Moves entities to targets, using pathfinding.
 *
 * Usage:
 *   Call moveTo() to start moving an entity towards a position or another
 *   entity. Each tick, the pathfinder finds a path and sets the entity's
 *   inputs to follow it (including jumping onto ledges). Check
 *   getMoveStatus() to see how it's going, and call stop() when you're done.
 *
 *   moveTo() can be called every tick with the same target, it only resets
 *   the entity's movement if the target changes.
 *
 * Entities must have the movement components (see World::addMovementComponents)
 * and a Collision component. Their size and climbing ability are derived from
 * their collision bounds and movement modifiers (see calcAgentProfile()).
 *
 * The underlying nav grid and request queue are also available, for
 * synchronous queries and for requesting paths without following them.
 */
class Pathfinder
{
public:
    /** The default value for moveTo()'s acceptanceRadius. */
    static constexpr float DEFAULT_ACCEPTANCE_RADIUS{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH) / 4.f};

    Pathfinder(entt::registry& inRegistry,
               CollisionLocator& inCollisionLocator);

    ~Pathfinder();

    //-------------------------------------------------------------------------
    // Engine Interface
    //-------------------------------------------------------------------------
    /**
     * Sizes and builds the nav grid to match the given map extent.
     * Call this after the tile map is loaded.
     */
    void init(const TileExtent& mapTileExtent);

    /**
     * Updates the nav grid, searches for paths, and updates the inputs of
     * all moving entities.
     * Call this once per tick, after AI and before movement.
     */
    void processPathFollowing();

    //-------------------------------------------------------------------------
    // Movement
    //-------------------------------------------------------------------------
    /**
     * Starts moving the given entity towards the given position.
     *
     * @param acceptanceRadius How close the entity must get to the position,
     *                         horizontally.
     * @return true if successful, else false (the entity is missing a
     *         required component).
     */
    bool moveTo(entt::entity entity, const Vector3& targetPosition,
                float acceptanceRadius = DEFAULT_ACCEPTANCE_RADIUS);

    /**
     * Starts moving the given entity towards the given target entity. The
     * entity will keep following the target as it moves.
     *
     * @param acceptanceRadius How close the entity must get to the target,
     *                         horizontally.
     * @return true if successful, else false (the entity is missing a
     *         required component).
     */
    bool moveTo(entt::entity entity, entt::entity targetEntity,
                float acceptanceRadius = DEFAULT_ACCEPTANCE_RADIUS);

    /**
     * Stops moving the given entity, and releases its inputs.
     */
    void stop(entt::entity entity);

    /**
     * Returns the given entity's movement status.
     */
    MoveStatus getMoveStatus(entt::entity entity) const;

    //-------------------------------------------------------------------------
    // Queries
    //-------------------------------------------------------------------------
    /**
     * Returns the nav grid, for synchronous queries.
     */
    const NavGrid& getNavGrid() const;

    /**
     * Returns the request queue, for requesting paths without following them.
     */
    NavRequestQueue& getRequestQueue();

    /**
     * Returns the given entity's agent profile, derived from its collision
     * bounds and movement modifiers.
     *
     * Note: The entity must have Collision and MovementModifiers components.
     */
    NavAgentProfile calcAgentProfile(entt::entity entity) const;

    /**
     * Returns the tile to use in path requests for an agent with the given
     * footprint, whose feet are at the given position (i.e. a Position
     * component).
     *
     * The agent must be standing on a surface (within a third of a tile). If
     * it isn't (e.g. it's in the air), returns nullopt.
     */
    std::optional<TilePosition> findNavTile(const Vector3& position,
                                            Uint8 footprintTiles) const;

private:
    /**
     * Starts or updates the given entity's movement. See moveTo().
     */
    bool startMoving(entt::entity entity, entt::entity targetEntity,
                     const Vector3& targetPosition, float acceptanceRadius);

    /**
     * Checks for arrival, and requests a new path if one is needed.
     * Runs before path requests are processed.
     */
    void planPath(entt::entity entity, PathFollow& pathFollow);

    /**
     * Picks up finished path requests and sets the entity's inputs to follow
     * its path.
     * Runs after path requests are processed.
     */
    void followPath(entt::entity entity, PathFollow& pathFollow);

    /**
     * Returns the position of the given follower's target, or nullopt if the
     * target entity no longer exists.
     */
    std::optional<Vector3>
        getTargetPosition(const PathFollow& pathFollow) const;

    /**
     * Sets the given follower's status, and if it's no longer moving, clears
     * its path and releases its inputs.
     */
    void setStatus(entt::entity entity, PathFollow& pathFollow,
                   MoveStatus status);

    /**
     * Releases the given follower's path request, if it has one.
     */
    void releaseRequest(PathFollow& pathFollow);

    /**
     * Returns the inputs that move an entity at the given position towards
     * the given target position.
     *
     * @param tolerance How close the entity must be along an axis before it
     *                  stops moving along that axis.
     */
    Input::StateArr calcSteeringInputs(const Vector3& position,
                                       const Vector3& target,
                                       float tolerance) const;

    /**
     * Returns true if an entity at the given position has reached the given
     * waypoint.
     */
    bool hasReachedWaypoint(const Vector3& position,
                            const NavWaypoint& waypoint, float tolerance) const;

    /**
     * Updates the given follower's stuck detection. See
     * SharedConfig::PATHFINDING_STUCK_TICKS.
     */
    void updateProgress(entt::entity entity, PathFollow& pathFollow,
                        const Position& position);

    /**
     * Sets the given entity's inputs, if they changed.
     * Note: Inputs are replicated when they change, so we avoid touching them
     *       otherwise.
     */
    void setInputs(entt::entity entity, const Input::StateArr& inputStates);

    /**
     * Returns the max height, in world units, that an entity with the given
     * jump impulse can jump.
     */
    float calcJumpHeight(float jumpImpulse) const;

    /**
     * Returns the horizontal distance between the given positions.
     */
    float calcHorizontalDistance(const Vector3& a, const Vector3& b) const;

    /**
     * Called when a PathFollow component is destroyed. Releases its request.
     */
    void onPathFollowDestroyed(entt::registry& registry, entt::entity entity);

    /** The registry that holds the entities that we move. */
    entt::registry& registry;

    /** The navigation data for the world. */
    NavGrid navGrid;

    /** Used to search for paths. */
    NavRequestQueue requestQueue;
};

} // End namespace AM
