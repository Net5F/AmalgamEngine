#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of entity interactions that the engine provides.
 *
 * Note: Don't use this enum directly, use EntityInteractionType (it combines
 *       the engine's and the project's interactions).
 */
enum class EngineEntityInteractionType : Uint8 {
    NotSet,

    // We reserve values 0 - 49. The project can start at 50.
    PROJECT_START = 50
};

} // End namespace AM
