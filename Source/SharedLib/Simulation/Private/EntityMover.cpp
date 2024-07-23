#include "EntityMover.h"
#include "TileMapBase.h"
#include "EntityLocator.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "Rotation.h"
#include "Collision.h"
#include "IsClientEntity.h"
#include "MinMaxBox.h"
#include "MovementHelpers.h"
#include "Transforms.h"
#include "Log.h"
#include "entt/entity/registry.hpp"

namespace AM
{
EntityMover::EntityMover(const entt::registry& inRegistry,
                         const TileMapBase& inTileMap,
                         EntityLocator& inEntityLocator)
: registry{inRegistry}
, tileMap{inTileMap}
, entityLocator{inEntityLocator}
, broadPhaseMatches{}
{
}

void EntityMover::moveEntity(
    entt::entity entity, const Input::StateArr& inputStates, Position& position,
    const PreviousPosition& previousPosition, Movement& movement,
    const MovementModifiers& movementMods, Rotation& rotation,
    Collision& collision, double deltaSeconds)
{
    // If no inputs are pressed and they aren't falling, nothing needs to 
    // be done.
    if (inputStates.none() && (movement.velocity.z == 0)) {
        movement.velocity = {0, 0, 0};
        return;
    }

    // Calculate their updated velocity.
    movement.velocity
        = MovementHelpers::calcVelocity(inputStates, movement, movementMods);

    // Resolve any collisions with the surrounding bounding boxes.
    BoundingBox resolvedBounds{resolveCollisions(collision.worldBounds, entity,
                                                 movement, deltaSeconds)};

    // Update their bounding box and position.
    // Note: The model bounds stage is centered on the position, not the 
    //       model bounds directly. Because of this, we can't just get 
    //       the position from the bottom center of the resolved bounds.
    position += (resolvedBounds.center - collision.worldBounds.center);
    // Note: Since clients calc bounds from the replicated position, we need to 
    //       use the same math here (instead of using resolvedBounds directly) 
    //       or the float result will end up slightly different.
    collision.worldBounds
        = Transforms::modelToWorldEntity(collision.modelBounds, position);

    // Update the direction they're facing, based on their current inputs.
    rotation = MovementHelpers::calcRotation(rotation, inputStates);

    // If they did actually move, update their position in the locator.
    if (position != previousPosition) {
        entityLocator.setEntityLocation(entity, collision.worldBounds);
    }
}

BoundingBox EntityMover::resolveCollisions(const BoundingBox& currentBounds,
                                           entt::entity movingEntity,
                                           Movement& movement,
                                           double deltaSeconds)
{
    broadPhaseMatches.clear();

    // Calc where the bounds will end up if there are no collisions.
    BoundingBox desiredBounds{currentBounds};
    desiredBounds.center
        += (movement.velocity * static_cast<float>(deltaSeconds));

    // Calc an extent that encompasses the entire potential movement.
    // Note: We add epsilon so that, if a box exactly lines up with the line  
    //       where two tiles meet, both tiles will be included.
    //       See the note in TileExtent(MinMaxBox) for info on why this isn't 
    //       the standard behavior.
    BoundingBox broadPhaseBounds{currentBounds};
    broadPhaseBounds.unionWith(desiredBounds);
    broadPhaseBounds.halfExtents.x += MovementHelpers::WORLD_EPSILON;
    broadPhaseBounds.halfExtents.y += MovementHelpers::WORLD_EPSILON;
    broadPhaseBounds.halfExtents.z += MovementHelpers::WORLD_EPSILON;

    // Clip the extent to the tile map's bounds.
    TileExtent broadPhaseTileExtent(broadPhaseBounds);
    broadPhaseTileExtent.intersectWith(tileMap.getTileExtent());

    // Collect the volumes of any tiles that intersect the broad phase bounds.
    for (int z{broadPhaseTileExtent.z}; z <= broadPhaseTileExtent.zMax(); ++z) {
        for (int y{broadPhaseTileExtent.y}; y <= broadPhaseTileExtent.yMax();
             ++y) {
            for (int x{broadPhaseTileExtent.x};
                 x <= broadPhaseTileExtent.xMax(); ++x) {
                // If this tile doesn't exist, it's empty so we can skip it.
                const Tile* tile{tileMap.cgetTile({x, y, z})};
                if (!tile) {
                    continue;
                }

                for (const BoundingBox& collisionVolume :
                     tile->getCollisionVolumes()) {
                    broadPhaseMatches.emplace_back(collisionVolume);
                }
            }
        }
    }

    // Collect the volumes of any non-client entities (besides the entity trying
    // to move) that intersect the broad phase bounds.
    std::vector<entt::entity>& entitiesBroadPhase{
        entityLocator.getEntitiesBroad(broadPhaseTileExtent)};
    for (entt::entity entity : entitiesBroadPhase) {
        if ((entity != movingEntity)
            && !(registry.all_of<IsClientEntity>(entity))) {
            const Collision& collision{registry.get<Collision>(entity)};
            broadPhaseMatches.emplace_back(collision.worldBounds);
        }
    }

    // Perform the iterations of the narrow phase to resolve any collisions.
    Vector3 originalVelocity{movement.velocity};
    BoundingBox resolvedBounds{currentBounds};
    float remainingTime{1.f};
    for (int i{0}; i < NARROW_PHASE_ITERATION_COUNT; ++i) {
        NarrowPhaseResult result{
            narrowPhase(resolvedBounds, movement, deltaSeconds, remainingTime)};
        resolvedBounds = result.resolvedBounds;
        remainingTime = result.remainingTime;

        if (result.remainingTime == 0) {
            break;
        }
    }

    // If the entity is in the air, maintain their X/Y velocity. This lets 
    // them continue moving, even if they temporarily get hung up on something
    // (e.g. hitting their feet on a wall while trying to jump onto it).
    // If the entity is grounded, this won't do anything (their X/Y velocity 
    // will be overwritten on the next tick).
    movement.velocity.x = originalVelocity.x;
    movement.velocity.y = originalVelocity.y;

    // If the final resolved bounds are outside of the map bounds, reject the 
    // move.
    TileExtent resolvedTileExtent(resolvedBounds);
    if (!tileMap.getTileExtent().containsExtent(resolvedTileExtent)) {
        return currentBounds;
    }

    return resolvedBounds;
}

EntityMover::NarrowPhaseResult
    EntityMover::narrowPhase(const BoundingBox& currentBounds,
                             Movement& movement, double deltaSeconds,
                             float remainingTime)
{
    // This is the real distance that we're trying to move on this frame.
    Vector3 realVelocity{movement.velocity * static_cast<float>(deltaSeconds)};

    // Find the time of the first collision.
    float collisionTime{remainingTime};
    Vector3 normalToUse{};
    for (const BoundingBox& otherBounds : broadPhaseMatches) {
        Vector3 entryDistance{};
        Vector3 exitDistance{};
        Vector3 entryTimes{-std::numeric_limits<float>::infinity(),
                           -std::numeric_limits<float>::infinity(),
                           -std::numeric_limits<float>::infinity()};
        Vector3 exitTimes{std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::infinity()};

        MinMaxBox currentBox{currentBounds};
        MinMaxBox otherBox{otherBounds};

        // Calc the distances required for resolvedBounds to enter and exit 
        // otherBounds along each axis, then calc the time intervals where 
        // each axis is intersecting.
        if (realVelocity.x > 0.f) {
            entryDistance.x = otherBox.min.x - currentBox.max.x;
            exitDistance.x = otherBox.max.x - currentBox.min.x;
            entryTimes.x = entryDistance.x / realVelocity.x;
            exitTimes.x = exitDistance.x / realVelocity.x;
        }
        else if (realVelocity.x < 0.f) {
            entryDistance.x = otherBox.max.x - currentBox.min.x;
            exitDistance.x = otherBox.min.x - currentBox.max.x;
            entryTimes.x = entryDistance.x / realVelocity.x;
            exitTimes.x = exitDistance.x / realVelocity.x;
        }
        // Velocity == 0. If this axis isn't intersecting, it never will.
        else if (currentBox.max.x <= otherBox.min.x
                 || currentBox.min.x >= otherBox.max.x) {
            continue;
        }
        // Else velocity == 0 and the boxes are intersecting. Entry/exit times 
        // are defaulted to (-inf, inf) to handle this case.

        if (realVelocity.y > 0.f) {
            entryDistance.y = otherBox.min.y - currentBox.max.y;
            exitDistance.y = otherBox.max.y - currentBox.min.y;
            entryTimes.y = entryDistance.y / realVelocity.y;
            exitTimes.y = exitDistance.y / realVelocity.y;
        }
        else if (realVelocity.y < 0.f) {
            entryDistance.y = otherBox.max.y - currentBox.min.y;
            exitDistance.y = otherBox.min.y - currentBox.max.y;
            entryTimes.y = entryDistance.y / realVelocity.y;
            exitTimes.y = exitDistance.y / realVelocity.y;
        }
        else if (currentBox.max.y <= otherBox.min.y
                 || currentBox.min.y >= otherBox.max.y) {
            continue;
        }

        if (realVelocity.z > 0.f) {
            entryDistance.z = otherBox.min.z - currentBox.max.z;
            exitDistance.z = otherBox.max.z - currentBox.min.z;
            entryTimes.z = entryDistance.z / realVelocity.z;
            exitTimes.z = exitDistance.z / realVelocity.z;
        }
        else if (realVelocity.z < 0.f) {
            entryDistance.z = otherBox.max.z - currentBox.min.z;
            exitDistance.z = otherBox.min.z - currentBox.max.z;
            entryTimes.z = entryDistance.z / realVelocity.z;
            exitTimes.z = exitDistance.z / realVelocity.z;
        }
        else if (currentBox.max.z <= otherBox.min.z
                 || currentBox.min.z >= otherBox.max.z) {
            continue;
        }

        // Determine if the time intervals ever overlap eachother within the 
        // range [0, remainingTime] (i.e. if the boxes ever intersect in all 3 
        // axes during our desired movement).
        float maxEntryTime{
            std::max({entryTimes.x, entryTimes.y, entryTimes.z})};
        float minExitTime{
            std::min({exitTimes.x, exitTimes.y, exitTimes.z})};

        // No-collision cases:
        //   1. If maxEntry > minExit, all axes haven't entered until after 
        //      one has already left.
        //   2. If all entry times are < 0, the boxes are either already 
        //      colliding or have passed eachother.
        //   3. If maxEntryTime > remainingTime, a collision won't happen 
        //      during this movement.
        if (maxEntryTime > minExitTime
            || (entryTimes.x < 0.f && entryTimes.y < 0.f && entryTimes.z < 0.f)
            || (entryTimes.x > remainingTime) || (entryTimes.y > remainingTime)
            || (entryTimes.z > remainingTime)) {
            continue;
        }

        // There was a collision. Find the axis of rejection by determining 
        // which axis collided last, then use the opposite sign of our velocity
        // along that axis to get a surface normal.
        Vector3 normal{};
        if ((entryTimes.x > entryTimes.y) && (entryTimes.x > entryTimes.z)) {
            normal.x = -std::copysign(1.f, realVelocity.x);
        }
        else if (entryTimes.y > entryTimes.z) {
            normal.y = -std::copysign(1.f, realVelocity.y);
        }
        else {
            normal.z = -std::copysign(1.f, realVelocity.z);
        }

        // If this collision time is the smallest so far, use it.
        if (maxEntryTime < collisionTime) {
            collisionTime = maxEntryTime;
            normalToUse = normal;
        }
    }

    // Move the bounds to resolve the collision.
    BoundingBox resolvedBounds{currentBounds};
    resolvedBounds.center += (realVelocity * collisionTime);

    // If they collided with the ground, reset their falling state.
    if (normalToUse.z == 1.f) {
        movement.jumpCount = 0;
    }

    // Set the velocity such that they'll slide along the collided surface 
    // on the next tick.
    movement.velocity = movement.velocity.slide(normalToUse);

    return {resolvedBounds, (remainingTime - collisionTime)};
}

} // End namespace AM
