#pragma once

#include "CollisionLocator.h"

namespace AM
{

// Raycast strategies. Defined in the CollisionLocator namespace so they have 
// access to the locator's private members.
struct CollisionLocator::RaycastStrategyIntersectAny
{
    RaycastStrategyIntersectAny(CollisionLocator& inCollisionLocator);

    bool isDone() const;

    void intersectObjectsInCell(const Vector3& start,
                                const Vector3& inverseRayDirection,
                                const CellPosition& cellPosition,
                                CollisionLayerBitSet collisionMask,
                                std::span<entt::entity> entitiesToExclude,
                                bool ignoreInsideHits);

    bool hasIntersected;

private:
    CollisionLocator& collisionLocator;
};

struct CollisionLocator::RaycastStrategyIntersectFirst
{
    RaycastStrategyIntersectFirst(CollisionLocator& inCollisionLocator);

    bool isDone() const;

    void intersectObjectsInCell(const Vector3& start,
                                const Vector3& inverseRayDirection,
                                const CellPosition& cellPosition,
                                CollisionLayerBitSet collisionMask,
                                std::span<entt::entity> entitiesToExclude,
                                bool ignoreInsideHits);

    bool hasIntersected;

    CollisionLocator::RaycastHitInfo firstHitInfo;

private:
    CollisionLocator& collisionLocator;
};

struct CollisionLocator::RaycastStrategyIntersectAll
{
    RaycastStrategyIntersectAll(CollisionLocator& inCollisionLocator);

    bool isDone() const;

    void intersectObjectsInCell(const Vector3& start,
                                const Vector3& inverseRayDirection,
                                const CellPosition& cellPosition,
                                CollisionLayerBitSet collisionMask,
                                std::span<entt::entity> entitiesToExclude,
                                bool ignoreInsideHits);

private:
    CollisionLocator& collisionLocator;
};

} // End namespace AM
