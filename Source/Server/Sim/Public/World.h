#pragma once

#include "SimDefs.h"
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

    /**
     * Returns a random spawn point position, with all points being within a
     * single AoI.
     */
    Position getRandomSpawnPoint();

    /**
     * Returns the next spawn point, trying to build groups of 10.
     */
    Position getGroupedSpawnPoint();

private:
    // For random spawn points.
    std::random_device device;
    std::mt19937 generator;
    std::uniform_real_distribution<float> xDistribution;
    std::uniform_real_distribution<float> yDistribution;

    // For grouped spawn points.
    static constexpr unsigned int GROUP_SIZE = 10;
    float baseX;
    float baseY;
    std::array<float, GROUP_SIZE> groupOffsetsX;
    std::array<float, GROUP_SIZE> groupOffsetsY;
    unsigned int offsetSelector;
};

} // namespace Server
} // namespace AM
