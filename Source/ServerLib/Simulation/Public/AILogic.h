#pragma once

#include "entt/fwd.hpp"

namespace AM
{
namespace Server
{
class World;

/**
 * Base class for entity AI logic.
 */
class AILogic {
public:
    /**
     * @param inEntity The entity that this AI is controlling.
     */
    AILogic(World& inWorld, entt::entity inEntity);

    /**
     * Processes one iteration of AI logic.
     *
     * If the given entity doesn't possess any of the necessary components, 
     * prints a warning and returns early.
     */
    virtual void tick() = 0;

protected:
    World& world;

    /** The entity that this AI is controlling. */
    entt::entity entity;
};

} // namespace Server
} // namespace AM
