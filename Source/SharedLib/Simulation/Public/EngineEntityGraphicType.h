#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * The entity graphic types that the engine provides.
 *
 * Note: Don't use this enum directly, use EntityGraphicType (it combines
 *       the engine's and the project's graphic types).
 */
enum class EngineEntityGraphicType : Uint8 {
    NotSet,
    IdleSouth,
    IdleSouthWest,
    IdleWest,
    IdleNorthWest,
    IdleNorth,
    IdleNorthEast,
    IdleEast,
    IdleSouthEast,
    RunSouth,
    RunSouthWest,
    RunWest,
    RunNorthWest,
    RunNorth,
    RunNorthEast,
    RunEast,
    RunSouthEast,

    // We reserve values 0 - 49. The project can start at 50.
    PROJECT_START = 50
};

} // End namespace AM
