#pragma once

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
     *
     * Updates velocity components based on input state, moves position
     * components based on velocity, updates sprites based on position.
     */
    void processMovements();

private:
    World& world;
};

} // namespace Server
} // namespace AM
