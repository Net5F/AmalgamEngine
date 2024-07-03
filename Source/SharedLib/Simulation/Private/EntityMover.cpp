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
{
}

void EntityMover::moveEntity(entt::entity entity,
                             const Input::StateArr& inputStates,
                             Position& position,
                             const PreviousPosition& previousPosition,
                             Movement& movement, Rotation& rotation,
                             Collision& collision, double deltaSeconds)
{
    // If no inputs are pressed and they aren't falling, nothing needs to 
    // be done.
    if (inputStates.none() && !(movement.isFalling)) {
        return;
    }

    // Calculate their updated velocity.
    Vector3 updatedVelocity{MovementHelpers::calcVelocity(
        inputStates, movement, SharedConfig::SIM_TICK_TIMESTEP_S)};

    // Calculate their desired next position.
    Position desiredPosition{MovementHelpers::calcPosition(
        position, updatedVelocity, SharedConfig::SIM_TICK_TIMESTEP_S)};

    // Update the direction they're facing, based on their current inputs.
    rotation = MovementHelpers::calcRotation(rotation, inputStates);

    // Calculate a new bounding box to match their desired position.
    BoundingBox desiredBounds{Transforms::modelToWorldEntity(
        collision.modelBounds, desiredPosition)};

    // Resolve any collisions with the surrounding bounding boxes.
    BoundingBox resolvedBounds{resolveCollisions(
        collision.worldBounds, desiredBounds, entity, movement)};

    // Update their bounding box and position.
    // Note: The model bounds stage is centered on the position, not the 
    //       model bounds directly. Because of this, we can't just get 
    //       the position from the bottom center of the resolved bounds.
    position += (resolvedBounds.center - collision.worldBounds.center);
    collision.worldBounds = resolvedBounds;

    // If they did actually move, update their position in the locator.
    if (position != previousPosition) {
        entityLocator.setEntityLocation(entity, collision.worldBounds);
    }
}

BoundingBox EntityMover::resolveCollisions(const BoundingBox& currentBounds,
                                           const BoundingBox& desiredBounds,
                                           entt::entity movingEntity,
                                           Movement& movement)
{
    detectedMatches.clear();

    // We check for vertical collision up to 1 tile above and below the desired 
    // bounds, being careful not to go outside the map.
    const TileExtent desiredTileExtent{desiredBounds.asTileExtent()};
    const TileExtent mapExtent{tileMap.getTileExtent()};
    int minZ{desiredTileExtent.z - 1};
    minZ = std::max(minZ, mapExtent.z);
    int maxZ{desiredTileExtent.zMax() + 1};
    maxZ = std::min(maxZ, mapExtent.zMax());

    // For each tile that the desired bounds is touching.
    for (int z{minZ}; z <= maxZ; ++z) {
        for (int y{desiredTileExtent.y}; y <= desiredTileExtent.yMax(); ++y) {
            for (int x{desiredTileExtent.x}; x <= desiredTileExtent.xMax();
                 ++x) {
                // If this tile doesn't exist, it's empty so we can skip it.
                const Tile* tile{tileMap.cgetTile({x, y, z})};
                if (!tile) {
                    continue;
                }

                // Collect any volumes that intersect the desired bounds.
                for (const BoundingBox& collisionVolume :
                     tile->getCollisionVolumes()) {
                    if (desiredBounds.intersects(collisionVolume)) {
                        detectedMatches.emplace_back(collisionVolume);
                    }
                }
            }
        }
    }

    // Collect the volumes of any non-client entities (besides the entity trying
    // to move) that intersect the desired bounds.
    std::vector<entt::entity>& entitiesBroadphase{
        entityLocator.getEntitiesBroad(desiredBounds)};
    for (entt::entity entity : entitiesBroadphase) {
        if ((entity != movingEntity)
            && !(registry.all_of<IsClientEntity>(entity))) {
            const Collision& collision{registry.get<Collision>(entity)};
            if (desiredBounds.intersects(collision.worldBounds)) {
                detectedMatches.emplace_back(collision.worldBounds);
            }
        }
    }

    // TODO: Don't forget to set isFalling
    // Resolve collision with all of the intersected volumes.
    BoundingBox resolvedBounds{desiredBounds};
    for (const BoundingBox& intersectedBounds : detectedMatches) {
        // TODO: Resolution
    }

    // If the final resolved bounds are outside of the map bounds, reject the 
    // move.
    TileExtent resolvedTileExtent{resolvedBounds.asTileExtent()};
    if (!mapExtent.containsExtent(resolvedTileExtent)) {
        return currentBounds;
    }

    return resolvedBounds;
}

} // End namespace AM
