#pragma once

#include "EntityMover.h"

namespace AM
{
namespace Server
{
class World;

/**
 * Moves entities.
 */
class MovementSystem
{
public:
    MovementSystem(World& inWorld);

    /**
     * Processes 1 tick of entity movement.
     */
    void processMovements();

private:
    World& world;

    EntityMover entityMover;
};

} // namespace Server
} // namespace AM
