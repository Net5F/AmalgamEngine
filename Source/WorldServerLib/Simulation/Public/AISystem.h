#pragma once

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class World;

/**
 * Handles AI processing.
 */
class AISystem
{
public:
    AISystem(const SimulationContext& inSimContext);

    /**
     * Calls tick() on all AI components.
     */
    void processAITick();

private:
    /** Used to get AI components to process. */
    World& world;
};

} // End namespace WorldServer
} // End namespace AM
