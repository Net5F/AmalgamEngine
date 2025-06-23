#include "AVEntityHelpers.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Client
{

std::optional<Position>
    AVEntityHelpers::getCasterPosition(entt::entity casterEntity,
                                       const entt::registry& registry)
{
    if (registry.valid(casterEntity)) {
        return registry.get<Position>(casterEntity);
    }

    // Caster is gone, don't spawn this entity.
    return {};
}

std::optional<Position> AVEntityHelpers::getTargetPosition(
    AVEntity::Behavior behavior, entt::entity targetEntity,
    const Position& targetPosition, const Position& currentPosition,
    const entt::registry& registry, bool isFirstPhase)
{
    switch (behavior) {
        case AVEntity::Behavior::MoveToEntity:
        case AVEntity::Behavior::FollowEntityStartCaster:
        case AVEntity::Behavior::FollowEntityStartTarget: {
            if (const auto* position{
                    registry.try_get<Position>(targetEntity)}) {
                return *position;
            }
            else {
                // Entity no longer exists.
                return {};
            }
        }
        case AVEntity::Behavior::MoveToPosition:
        case AVEntity::Behavior::FollowDirection:
        case AVEntity::Behavior::StaticPosition:
            return targetPosition;
        case AVEntity::Behavior::CurrentPosition: {
            if (!isFirstPhase) {
                return currentPosition;
            }
            else {
                break;
            }
        }
    }

    return {};
}

std::optional<AVEntityHelpers::StartStateReturn> AVEntityHelpers::getStartState(
    AVEntity::Behavior behavior, const Position& casterPosition,
    const Position& targetPosition, float startDistance)
{
    switch (behavior) {
        case AVEntity::Behavior::MoveToEntity:
        case AVEntity::Behavior::FollowEntityStartCaster:
        case AVEntity::Behavior::MoveToPosition:
        case AVEntity::Behavior::FollowDirection: {
            // Calculate a direction vector that points towards the target.
            Vector3 directionVec{targetPosition - casterPosition};
            directionVec.normalize();

            // Start at the front of the caster, offset towards the target.
            return StartStateReturn{
                casterPosition + (directionVec * startDistance),
                EntityGraphicType::Run, vectorToClosestDirection(directionVec)};
        }
        case AVEntity::Behavior::FollowEntityStartTarget:
        case AVEntity::Behavior::StaticPosition:
            return StartStateReturn{targetPosition, EntityGraphicType::Idle,
                                    Rotation::Direction::South};
        case AVEntity::Behavior::CurrentPosition:
            // CurrentPosition is invalid as a first phase.
            return {};
    }

    return {};
}

std::optional<AVEntityHelpers::GraphicStateReturn>
    AVEntityHelpers::getGraphicState(AVEntity::Behavior behavior,
                                     const Position& casterPosition,
                                     const Position& targetPosition)
{
    switch (behavior) {
        case AVEntity::Behavior::MoveToEntity:
        case AVEntity::Behavior::FollowEntityStartCaster:
        case AVEntity::Behavior::MoveToPosition:
        case AVEntity::Behavior::FollowDirection: {
            Vector3 directionVec{targetPosition - casterPosition};
            directionVec.normalize();
            return GraphicStateReturn{EntityGraphicType::Run,
                                      vectorToClosestDirection(directionVec)};
        }
        case AVEntity::Behavior::FollowEntityStartTarget:
        case AVEntity::Behavior::StaticPosition:
            return GraphicStateReturn{EntityGraphicType::Idle,
                                      Rotation::Direction::South};
    }

    return {};
}

bool AVEntityHelpers::timeElapsed(AVEntity::Behavior behavior, double startTime,
                                  double durationS, double currentTime)
{
    switch (behavior) {
        case AVEntity::Behavior::FollowEntityStartCaster:
        case AVEntity::Behavior::FollowDirection:
        case AVEntity::Behavior::FollowEntityStartTarget:
        case AVEntity::Behavior::StaticPosition: {
            bool entityStarted{startTime != 0};
            double endTime{startTime + durationS};
            if (entityStarted && (currentTime >= endTime)) {
                return true;
            }
        }
    }

    return false;
}

bool AVEntityHelpers::positionReached(AVEntity::Behavior behavior,
                                      const Position& entityPos,
                                      const Position& targetPos)
{
    // If this is a position-based phase and we've reached the target, 
    // return true.
    switch (behavior) {
        case AVEntity::Behavior::MoveToEntity:
        case AVEntity::Behavior::MoveToPosition: {
            if (entityPos == targetPos) {
                return true;
            }
        }
    }

    return false;
}

Rotation::Direction
    AVEntityHelpers::vectorToClosestDirection(const Vector3& directionVector)
{
    // Drop the Z axis since direction is 2D.
    Vector3 directionVector2D{directionVector.x, directionVector.y, 0};
    directionVector2D.normalize();

    // Multiply and round so that each axis has a value of -1, 0, or 1.
    // E.g. for the X axis, -1 == West, 0 == Center, 1 == East.
    int x{static_cast<int>(std::lround(directionVector2D.x))};
    int y{static_cast<int>(std::lround(directionVector2D.y))};

    // Return the appropriate direction.
    if (x == -1 && y == -1) {
        return Rotation::Direction::NorthWest;
    }
    else if (x == -1 && y == 0) {
        return Rotation::Direction::West;
    }
    else if (x == -1 && y == 1) {
        return Rotation::Direction::SouthWest;
    }
    else if (x == 0 && y == -1) {
        return Rotation::Direction::North;
    }
    else if (x == 0 && y == 1) {
        return Rotation::Direction::South;
    }
    else if (x == 1 && y == -1) {
        return Rotation::Direction::NorthEast;
    }
    else if (x == 1 && y == 0) {
        return Rotation::Direction::East;
    }
    else if (x == 1 && y == 1) {
        return Rotation::Direction::SouthEast;
    }

    // Direction vector is (0, 0). Default to South.
    return Rotation::Direction::South;
}

} // End namespace Client
} // End namespace AM
