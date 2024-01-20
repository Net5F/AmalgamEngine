#pragma once

#include "entt/fwd.hpp"

namespace AM
{
namespace Server
{
class World;

/**
 * Interface class for entity AI logic.
 */
class AILogic
{
public:
    // Note: We pass world and entity as parameters so that we don't have to 
    //       deal with a World& member during serialization.
    /**
     * Processes one iteration of AI logic.
     *
     * If the given entity doesn't possess any of the necessary components,
     * prints a warning and returns early.
     *
     * @param entity The entity that this AI is controlling.
     */
    virtual void tick(World& world, entt::entity entity) = 0;
};

} // namespace Server
} // namespace AM
