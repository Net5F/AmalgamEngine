#pragma once

#include "GameDefs.h"
#include "Position.h"

#include "entt/entity/registry.hpp"

#include <random>

namespace AM
{
namespace Server
{

/**
 * Holds world state and manages the persistence of that state.
 *
 * Also provides helpers for common uses of world state.
 */
class World
{
public:
    World();

    /** Entity data registry. */
    entt::registry registry;

    /**
     * Searches all entities with ClientComponents to find one associated with
     * the given networkID.
     * @return The EntityID of the entity associated with networkID if found,
     *         else INVALID_ENTITY_ID.
     */
    entt::entity findEntityWithNetID(NetworkID networkID);

    Position getSpawnPoint();

private:
    // Temp: Putting entities at random positions within the screen bounds.
    std::random_device device;
    std::mt19937 generator;
    std::uniform_real_distribution<float> xDistribution;
    std::uniform_real_distribution<float> yDistribution;
};

} // namespace Server
} // namespace AM
