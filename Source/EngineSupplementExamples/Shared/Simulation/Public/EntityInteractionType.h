#pragma once

#include "EngineEntityInteractionType.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of interactions that a user may be able to perform on an entity.
 */
enum class EntityInteractionType : Uint8 {
    // Engine interactions (copied here so we can use one strongly-typed enum).
    NotSet = static_cast<Uint8>(EngineEntityInteractionType::NotSet),
};

} // End namespace AM
