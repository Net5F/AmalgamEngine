#pragma once

#include "Input.h"
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
 */
class EntityMover {
public:
    EntityMover(const entt::registry& inRegistry, const TileMapBase& inTileMap,
                EntityLocator& inEntityLocator);

    void moveEntity(entt::entity entity, const Input::StateArr& inputStates,
                    Position& position,
                    const PreviousPosition& previousPosition,
                    Movement& movement, Rotation& rotation,
                    Collision& collision, double deltaSeconds);

private:
    const entt::registry& registry;
    const TileMapBase& tileMap;
    EntityLocator& entityLocator;
};

} // namespace AM
