#pragma once

#include "AVEntity.h"
#include "Position.h"
#include "Rotation.h"
#include "entt/entity/registry.hpp"
#include <optional>

namespace AM
{
namespace Client
{
/**
 * Shared static functions for working with A/V entities.
 */
class AVEntityHelpers
{
public:
    /**
     * If casterEntity is valid, returns its position. Else, returns null.
     */
    static std::optional<Position>
        getCasterPosition(entt::entity casterEntity,
                          const entt::registry& registry);

    /**
     * Returns the appropriate target position for the given behavior, either 
     * targetEntity's position or targetPosition.
     */
    static std::optional<Position> getTargetPosition(
        AVEntity::Behavior behavior, entt::entity targetEntity,
        const Position& targetPosition, const Position& currentPosition,
        const entt::registry& registry, bool isFirstPhase);

    struct StartStateReturn {
        Position startPosition{};
        EntityGraphicType graphicType{};
        Rotation::Direction graphicDirection{};
    };
    /**
     * Returns the appropriate starting position and graphic state for the 
     * given behavior.
     */
    static std::optional<StartStateReturn>
        getStartState(AVEntity::Behavior behavior,
                      const Position& casterPosition,
                      const Position& targetPosition, float startDistance);

    struct GraphicStateReturn {
        EntityGraphicType graphicType{};
        Rotation::Direction graphicDirection{};
    };
    /**
     * Returns the appropriate graphic state for the given behavior.
     */
    static std::optional<GraphicStateReturn>
        getGraphicState(AVEntity::Behavior behavior,
                        const Position& casterPosition,
                        const Position& targetPosition);

    /**
     * If the given behavior is time-based and its time has elapsed, returns 
     * true.
     */
    static bool timeElapsed(AVEntity::Behavior behavior, double startTime,
                            double durationS, double currentTime,
                            GraphicRef currentGraphic);

    /**
     * If the given behavior is position-based and entityPos == targetPos, 
     * returns true.
     */
    static bool positionReached(AVEntity::Behavior behavior,
                                const Position& entityPos,
                                const Position& targetPos);

    /**
     * Converts the given direction vector to the closest X/Y direction 
     * value. Ignores the Z axis.
     */
    static Rotation::Direction
        vectorToClosestDirection(const Vector3& directionVector);
};

} // End namespace Client
} // End namespace AM

