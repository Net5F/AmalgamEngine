#pragma once

#include "Input.h"
#include "BoundingBox.h"
#include "CollisionLocator.h"
#include "entt/fwd.hpp"

namespace AM
{
class TileMapBase;
class EntityLocator;
struct Position;
struct PreviousPosition;
struct Movement;
struct MovementModifiers;
struct Rotation;
struct Collision;

/**
 * A helper class that manages the logic of moving entities, resolving 
 * collisions, etc.
 * 
 * This is a class instead of a set of free functions to avoid needing to pass 
 * dependencies on every call.
 *
 * Note: Since player movement is predicted on clients, a player colliding with 
 *       a moving NPC is always going to cause mis-predictions and rollback.
 *       To avoid this, we have a blanket rule of "a moving entity (player or 
 *       NPC) will ignore the collision of all movement-enabled entities
 *       (whether they're currently moving or not)".
 */
class EntityMover {
public:
    EntityMover(const entt::registry& inRegistry, const TileMapBase& inTileMap,
                EntityLocator& inEntityLocator,
                CollisionLocator& inCollisionLocator);

    /**
     * Processes a tick of entity movement, updating the given components 
     * appropriately.
     */
    void moveEntity(entt::entity entity, const Input::StateArr& inputStates,
                    Position& position,
                    const PreviousPosition& previousPosition,
                    Movement& movement, const MovementModifiers& movementMods,
                    Rotation& rotation, Collision& collision,
                    double deltaSeconds);

private:
    /**
     * The maximum number of iterations that the narrow phase of collision 
     * resolution should perform.
     */
    static constexpr int NARROW_PHASE_ITERATION_COUNT{3};

    /**
     * Resolves collisions between the given desiredBounds and other nearby
     * bounding boxes in the world.
     *
     * @param currentBounds The bounding box, at its current position.
     * @param movement The entity's movement component.
     *
     * @return The desired bounding box, moved to resolve collisions.
     */
    BoundingBox resolveCollisions(const BoundingBox& currentBounds,
                                  Movement& movement, double deltaSeconds);

    struct NarrowPhaseResult {
        BoundingBox resolvedBounds{};
        float remainingTime{};
    };
    /**
     * Performs a single iteration of narrow phase collision resolution between 
     * the given bounds and all volumes in broadPhaseMatches.
     */
    NarrowPhaseResult
        narrowPhase(const std::vector<const CollisionLocator::CollisionInfo*>&
                        broadPhaseMatches,
                    const BoundingBox& currentBounds, Movement& movement,
                    double deltaSeconds, float remainingTime);

    const entt::registry& registry;
    const TileMapBase& tileMap;
    EntityLocator& entityLocator;
    CollisionLocator& collisionLocator;
};

} // namespace AM
