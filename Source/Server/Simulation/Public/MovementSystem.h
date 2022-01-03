#pragma once

namespace AM
{
namespace Server
{
class World;

/**
 * This system is in charge of moving entities.
 */
class MovementSystem
{
public:
    MovementSystem(World& inWorld);

    /**
     * Moves the all entities 1 sim tick into the future.
     * Updates velocity components based on input state, moves position
     * components based on velocity, updates sprites based on position.
     */
    void processMovements();

private:
    World& world;
};

} // namespace Server
} // namespace AM
