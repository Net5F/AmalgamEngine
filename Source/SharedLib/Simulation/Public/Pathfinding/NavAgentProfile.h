#pragma once

#include "SharedConfig.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Describes the size and movement abilities of a pathfinding agent.
 *
 * The defaults match the common agent profile from SharedConfig.
 *
 * Heights are measured in "thirds" of a tile (see NavSurface.h).
 */
struct NavAgentProfile {
    /** The max width, in tiles, of an agent's footprint. */
    static constexpr Uint8 MAX_FOOTPRINT_TILES{7};

    /** The width, in tiles, of the agent's square footprint.
        Must be between 1 and MAX_FOOTPRINT_TILES. */
    Uint8 footprintTiles{SharedConfig::PATHFINDING_COMMON_FOOTPRINT_TILES};

    /** The agent's height, in thirds. */
    Uint8 heightThirds{SharedConfig::PATHFINDING_COMMON_HEIGHT_THIRDS};

    /** The max height, in thirds, that the agent can climb (by jumping) when
        moving from one tile to the next. */
    Uint8 climbThirds{SharedConfig::PATHFINDING_COMMON_CLIMB_THIRDS};

    /** The max height, in thirds, that the agent will drop when moving from
        one tile to the next. */
    Uint8 maxDropThirds{SharedConfig::PATHFINDING_COMMON_MAX_DROP_THIRDS};
};

} // End namespace AM
