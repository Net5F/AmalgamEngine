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
    //LOG_INFO("Velocity before:");
    //movement.velocity.print();
    //LOG_INFO("Bounds before:");
    //collision.worldBounds.print();
    // If no inputs are pressed and they aren't falling, nothing needs to 
    // be done.
    if (inputStates.none() && !(movement.isFalling)) {
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
        //LOG_INFO("Inputs: %u, New position:", inputStates);
        //position.print();
        //LOG_INFO("Velocity after:");
        //movement.velocity.print();
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
    BoundingBox broadPhaseBounds{currentBounds};
    broadPhaseBounds.unionWith(desiredBounds);
    TileExtent broadPhaseTileExtent{broadPhaseBounds.asTileExtent()};

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
        entityLocator.getEntitiesBroad(broadPhaseBounds)};
    for (entt::entity entity : entitiesBroadPhase) {
        if ((entity != movingEntity)
            && !(registry.all_of<IsClientEntity>(entity))) {
            const Collision& collision{registry.get<Collision>(entity)};
            broadPhaseMatches.emplace_back(collision.worldBounds);
        }
    }

    // Perform the iterations of the narrow phase to resolve any collisions.
    BoundingBox resolvedBounds{currentBounds};
    float remainingTime{1.f};
    for (int i{0}; i < 3; ++i) {
        NarrowPhaseResult result{
            narrowPhase(resolvedBounds, movement, deltaSeconds, remainingTime)};
        resolvedBounds = result.resolvedBounds;
        remainingTime = result.remainingTime;

        if (!(result.didCollide) || (result.remainingTime == 0)) {
            break;
        }
    }

    // If the final resolved bounds are outside of the map bounds, reject the 
    // move.
    TileExtent resolvedTileExtent{resolvedBounds.asTileExtent()};
    if (!tileMap.getTileExtent().containsExtent(resolvedTileExtent)) {
        LOG_INFO("Outside map bounds");
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
    bool didCollide{false};
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
            //LOG_INFO("Zero velocity, X not intersecting. Early bail.");
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
            //LOG_INFO("Zero velocity, Y not intersecting. Early bail.");
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
            //LOG_INFO("Zero velocity, Z not intersecting. Early bail.");
            continue;
        }

        //LOG_INFO("Velocity");
        //realVelocity.print();
        //LOG_INFO("Entity box:");
        //currentBox.print();
        //LOG_INFO("Other box:");
        //otherBox.print();
        //LOG_INFO("Entry times:");
        //entryTimes.print();
        //LOG_INFO("Exit times:");
        //exitTimes.print();
        //LOG_INFO("RemainingTime: %.4f", remainingTime);

        // Determine if the time intervals ever overlap within the 0 - 1 range
        // (i.e. if the boxes ever intersect in all 3 axes during our desired 
        // movement).
        float maxEntryTime{
            std::max({entryTimes.x, entryTimes.y, entryTimes.z})};
        float minExitTime{
            std::min({exitTimes.x, exitTimes.y, exitTimes.z})};

        // No-collision cases:
        //   1. If maxEntry > minExit, all axes haven't entered until after 
        //      one has already left.
        //   2. If all entry times are <= 0, the boxes are either already 
        //      colliding or have passed eachother.
        //   3. If maxEntryTime > remainingTime, a collision won't happen 
        //      during this movement.
        // If there was no collision, take the full movement.
        Vector3 normal{};
        if (maxEntryTime > minExitTime
            || (entryTimes.x < 0.f && entryTimes.y < 0.f && entryTimes.z < 0.f)
            || (entryTimes.x > remainingTime) || (entryTimes.y > remainingTime)
            || (entryTimes.z > remainingTime)) {
            // collisionTime is already set to 1.
            //LOG_INFO("No collision");
            continue;
        }
        else {
            //LOG_INFO("Collided");
            // TODO: Do we need these != 0 checks and else?
            if (realVelocity.x != 0 && entryTimes.x > entryTimes.y
                && entryTimes.x > entryTimes.z) {
                if (entryDistance.x < 0.f) {
                    normal.x = 1.f;
                }
                else {
                    normal.x = -1.f;
                }
            }
            else if (realVelocity.y != 0 && entryTimes.y > entryTimes.z) {
                if (entryDistance.y < 0.f) {
                    normal.y = 1.f;
                }
                else {
                    normal.y = -1.f;
                }
            }
            else if (realVelocity.z != 0) {
                if (entryDistance.z < 0.f) {
                    normal.z = 1.f;
                }
                else {
                    normal.z = -1.f;
                }
            }
            else {
                continue;
            }
            didCollide = true;
        }

        // We collided. If this collision time is the smallest so far, use it.
        if (maxEntryTime < collisionTime) {
            collisionTime = maxEntryTime;
            normalToUse = normal;
        }
    }

    BoundingBox resolvedBounds{currentBounds};
    resolvedBounds.center += (realVelocity * collisionTime);
    if (normalToUse.x != 0) {
        //LOG_INFO("Resolving X");
    }
    else if (normalToUse.y != 0) {
        //LOG_INFO("Resolving Y");
    }
    else if (normalToUse.z != 0) {
        //LOG_INFO("Resolving Z");
        movement.isFalling = false;
        //LOG_INFO("isFalling = %u", movement.isFalling);
        movement.jumpCount = 0;
    }
    //LOG_INFO("Current bounds:");
    //currentBounds.print();
    //LOG_INFO("Resolved bounds:");
    //resolvedBounds.print();
    //LOG_INFO("RemainingTime: %.4f", (remainingTime - collisionTime));

    movement.velocity = movement.velocity.slide(normalToUse);

    //LOG_INFO("Normal:");
    //normal.print();
    //LOG_INFO("Slide vector:");
    //movement.velocity.print();
    //LOG_INFO("Slid bounds:");
    //resolvedBounds.print();

    // TODO: We don't really need didCollide, we can just use all the time
    return {resolvedBounds, didCollide, (remainingTime - collisionTime)};
}

} // End namespace AM
