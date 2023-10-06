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
     * Calls tick() on all AIBehavior components.
     */
    void processAITick();

private:
    /** Used to get AIBehavior components to process. */
    World& world;
};

} // End namespace Server
} // End namespace AM
