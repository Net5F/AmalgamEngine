#pragma once

#include <SDL_stdinc.h>

namespace AM
{
/**
 * RAII wrapper for SDL.
 */
class SDL
{
public:
    /**
     * Initializes SDL with the given parameters.
     *
     * On init failure, prints the error and aborts.
     */
	SDL(Uint32 flags);

    ~SDL();
};

} // namespace AM
