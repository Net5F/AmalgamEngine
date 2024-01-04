#pragma once

namespace AM
{
namespace Server
{

class World;

/**
 * Handles AI processing.
 */
class AISystem
{
public:
    AISystem(World& inWorld);

    /**
     * Calls tick() on all AI components.
     */
    void processAITick();

private:
    /** Used to get AI components to process. */
    World& world;
};

} // End namespace Server
} // End namespace AM
