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
    /** Note: All graphic sets are expected to contain IdleSouth. */
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
    CrouchSouth,
    CrouchSouthWest,
    CrouchWest,
    CrouchNorthWest,
    CrouchNorth,
    CrouchNorthEast,
    CrouchEast,
    CrouchSouthEast,
    JumpSouth,
    JumpSouthWest,
    JumpWest,
    JumpNorthWest,
    JumpNorth,
    JumpNorthEast,
    JumpEast,
    JumpSouthEast,
    PROJECT_START
};

} // End namespace AM
