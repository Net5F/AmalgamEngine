#include "MovementHelpers.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "BoundingBox.h"
#include "TileMapBase.h"
#include "EntityLocator.h"
#include "IsClientEntity.h"
#include "SharedConfig.h"
#include "entt/entity/registry.hpp"

/** The constant to multiply by when normalizing a diagonal direction vector
    to be equal magnitude to movement in cardinal directions.
    sin(45) == cos(45) == 0.70710678118 */
const float DIAGONAL_NORMALIZATION_CONSTANT{0.70710678118f};

namespace AM
{
Position MovementHelpers::calcPosition(const Position& position,
                                       const Input::StateArr& inputStates,
                                       double deltaSeconds)
{
    // Direction values. 0 == no movement, 1 == movement.
    int xUp{static_cast<int>(inputStates[Input::XUp])};
    int xDown{static_cast<int>(inputStates[Input::XDown])};
    int yUp{static_cast<int>(inputStates[Input::YUp])};
    int yDown{static_cast<int>(inputStates[Input::YDown])};
    int zUp{static_cast<int>(inputStates[Input::ZUp])};
    int zDown{static_cast<int>(inputStates[Input::ZDown])};

    // Calculate our direction vector, based on the entity's inputs.
    // Note: Opposite inputs cancel eachother out.
    float xDirection{static_cast<float>(xUp - xDown)};
    float yDirection{static_cast<float>(yUp - yDown)};
    float zDirection{static_cast<float>(zUp - zDown)};

    // If we're moving diagonally, normalize our direction vector.
    if ((xDirection != 0) && (yDirection != 0)) {
        xDirection *= DIAGONAL_NORMALIZATION_CONSTANT;
        yDirection *= DIAGONAL_NORMALIZATION_CONSTANT;
    }

    // Calc the velocity.
    float velocityX{xDirection * SharedConfig::MOVEMENT_VELOCITY};
    float velocityY{yDirection * SharedConfig::MOVEMENT_VELOCITY};
    float velocityZ{zDirection * SharedConfig::MOVEMENT_VELOCITY};

    // Update the position.
    Position newPosition{position};
    newPosition.x += static_cast<float>((deltaSeconds * velocityX));
    newPosition.y += static_cast<float>((deltaSeconds * velocityY));
    newPosition.z += static_cast<float>((deltaSeconds * velocityZ));

    return newPosition;
}

Rotation MovementHelpers::calcRotation(const Rotation& rotation,
                                       const Input::StateArr& inputStates)
{
    // Direction values. 0 == no movement, 1 == movement.
    int xUp{static_cast<int>(inputStates[Input::XUp])};
    int xDown{static_cast<int>(inputStates[Input::XDown])};
    int yUp{static_cast<int>(inputStates[Input::YUp])};
    int yDown{static_cast<int>(inputStates[Input::YDown])};

    // Calculate which direction the entity is facing, based on its inputs.
    // Note: Opposite inputs cancel eachother out.
    int directionInt{3 * (yDown - yUp) + xUp - xDown};
    Rotation::Direction direction{directionIntToDirection(directionInt)};

    switch (direction) {
        case Rotation::Direction::None: {
            // No inputs or canceling inputs, keep the current direction.
            return rotation;
        }
        default: {
            return {direction};
        }
    }
}

Position
    MovementHelpers::interpolatePosition(const PreviousPosition& previousPos,
                                         const Position& position, double alpha)
{
    double interpX{(position.x * alpha) + (previousPos.x * (1.0 - alpha))};
    double interpY{(position.y * alpha) + (previousPos.y * (1.0 - alpha))};
    double interpZ{(position.z * alpha) + (previousPos.z * (1.0 - alpha))};
    return {static_cast<float>(interpX), static_cast<float>(interpY),
            static_cast<float>(interpZ)};
}

BoundingBox MovementHelpers::resolveCollisions(const BoundingBox& currentBounds,
                                               const BoundingBox& desiredBounds,
                                               entt::entity movingEntity,
                                               const entt::registry& registry,
                                               const TileMapBase& tileMap,
                                               EntityLocator& entityLocator)
{
    // TODO: Replace this logic with real sliding collision.

    // If the desired movement would go outside of the map, reject the move.
    const TileExtent boxTileExtent{desiredBounds.asTileExtent()};
    const TileExtent mapExtent{tileMap.getTileExtent()};
    if (!mapExtent.containsExtent(boxTileExtent)) {
        return currentBounds;
    }

    // Check for vertical collision up to 1 tile above and below the bounds.
    int minZ{boxTileExtent.z - 1};
    minZ = std::max(minZ, mapExtent.z);
    int maxZ{boxTileExtent.zMax() + 1};
    maxZ = std::min(maxZ, mapExtent.zMax());

    // For each tile that the desired bounds is touching.
    for (int z{minZ}; z <= maxZ; ++z) {
        for (int y{boxTileExtent.y}; y <= boxTileExtent.yMax(); ++y) {
            for (int x{boxTileExtent.x}; x <= boxTileExtent.xMax(); ++x) {
                // If this tile doesn't exist, it's empty so we can skip it.
                const Tile* tile{tileMap.cgetTile({x, y, z})};
                if (!tile) {
                    continue;
                }

                // For each collision box in this tile.
                for (const BoundingBox& collisionBox :
                     tile->getCollisionBoxes()) {
                    // If the desired movement would intersect this box, reject
                    // the move.
                    if (desiredBounds.intersects(collisionBox)) {
                        return currentBounds;
                    }
                }
            }
        }
    }

    // If any non-client entity (besides the entity trying to move) intersects
    // the desired bounds, reject the move.
    std::vector<entt::entity>& collidedEntities{
        entityLocator.getCollisions(desiredBounds)};
    for (entt::entity collidedEntity : collidedEntities) {
        if ((collidedEntity != movingEntity)
            && !(registry.all_of<IsClientEntity>(collidedEntity))) {
            return currentBounds;
        }
    }

    return desiredBounds;
}

Rotation::Direction MovementHelpers::directionIntToDirection(int directionInt)
{
    switch (directionInt) {
        case -4: {
            return Rotation::Direction::SouthWest;
        }
        case -3: {
            return Rotation::Direction::South;
        }
        case -2: {
            return Rotation::Direction::SouthEast;
        }
        case -1: {
            return Rotation::Direction::West;
        }
        case 0: {
            return Rotation::Direction::None;
        }
        case 1: {
            return Rotation::Direction::East;
        }
        case 2: {
            return Rotation::Direction::NorthWest;
        }
        case 3: {
            return Rotation::Direction::North;
        }
        case 4: {
            return Rotation::Direction::NorthEast;
        }
        default: {
            LOG_FATAL("Invalid direction int.");
            return Rotation::Direction::None;
        }
    }
}

} // End namespace AM
