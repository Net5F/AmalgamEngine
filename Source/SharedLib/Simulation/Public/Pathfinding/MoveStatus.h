#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * The state of an entity's movement towards a target. See Pathfinder.
 */
enum class MoveStatus : Uint8 {
    /** The entity isn't being moved by the pathfinder. */
    None,
    /** The entity is moving towards its target (or waiting for a path). */
    Moving,
    /** The entity is within range of its target.
        If the target is an entity and it moves out of range, the entity will
        start moving again. */
    Arrived,
    /** There's no path to the target, or the target entity no longer
        exists.
        If the target is an entity and it moves, the entity will try again. */
    Unreachable,
    /** The entity couldn't make progress along its path, even after jumping
        and finding a new path. */
    Stuck
};

} // End namespace AM
