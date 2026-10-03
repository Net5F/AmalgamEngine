#pragma once

#include "EntityMover.h"

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class World;

/**
 * Moves entities.
 */
class MovementSystem
{
public:
    MovementSystem(const SimulationContext& inSimContext);

    /**
     * Processes 1 tick of entity movement.
     */
    void processMovements();

private:
    World& world;

    EntityMover entityMover;
};

} // namespace WorldServer
} // namespace AM
