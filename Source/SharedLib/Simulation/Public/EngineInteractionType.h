#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of interactions that a user may be able to perform on an entity.
 *
 * Note: We built this construct in anticipation of being able to handle a 
 *       worthwhile amount of interaction logic in the engine. If it doesn't 
 *       end up being useful and all the interactions are handled by the project,
 *       we could make things a lot simpler by getting rid of this.
 */
struct EngineInteractionType {
    enum Value : Uint8 {
        /** Indicates the value hasn't been set. Used for initialization. */
        NotSet,
        Temp,

        // We reserve values 0 - 124. The project can start at 125.
        PROJECT_START = 125
    };
};

} // End namespace AM
