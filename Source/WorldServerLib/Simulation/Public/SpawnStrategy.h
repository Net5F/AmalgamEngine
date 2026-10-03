#pragma once

namespace AM
{
namespace WorldServer
{

/**
 * The strategies that we can use for determining where entities should spawn.
 */
enum class SpawnStrategy {
    /** Spawn in a fixed location. */
    Fixed,
    /** Spawn in groups. */
    Grouped,
    /** Spawn in a random location. */
    Random
};

} // namespace WorldServer
} // namespace AM
