#pragma once

#include "Input.h"
#include "BoundingBox.h"
#include "entt/fwd.hpp"

namespace AM
{
class TileMapBase;
class EntityLocator;
struct Position;
struct PreviousPosition;
struct Movement;
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
 *       This is fine for the player experience, since devs should know to not 
 *       give collision to NPCs that move. It also means that we can get away 
 *       with not considering the moving NPC's velocity when resolving 
 *       collisions.
 */
class EntityMover {
public:
    EntityMover(const entt::registry& inRegistry, const TileMapBase& inTileMap,
                EntityLocator& inEntityLocator);

    /**
     * Processes a tick of entity movement, updating the given components 
     * appropriately.
     */
    void moveEntity(entt::entity entity, const Input::StateArr& inputStates,
                    Position& position,
                    const PreviousPosition& previousPosition,
                    Movement& movement, Rotation& rotation,
                    Collision& collision, double deltaSeconds);

private:
    /**
     * Resolves collisions between the given desiredBounds and other nearby
     * bounding boxes in the world.
     *
     * @param currentBounds The bounding box, at its current position.
     * @param desiredBounds The bounding box, at its desired position.
     * @param movingEntity The entity that's trying to move.
     * @param movement The entity's movement component.
     *
     * @return The desired bounding box, moved to resolve collisions.
     */
    BoundingBox resolveCollisions(const BoundingBox& currentBounds,
                                  const BoundingBox& desiredBounds,
                                  entt::entity movingEntity,
                                  Movement& movement);

    const entt::registry& registry;
    const TileMapBase& tileMap;
    EntityLocator& entityLocator;

    /** Scratch vector for holding bounds that intersect the desired bounds 
        during collision detection. */
    std::vector<BoundingBox> detectedMatches;
};

} // namespace AM
