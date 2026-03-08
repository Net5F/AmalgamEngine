#pragma once

#include <SDL3/SDL_stdinc.h>

struct SDL_Window;
struct SDL_Renderer;

namespace AM
{
/**
 * RAII wrapper for SDL_Renderer.
 */
class SDLRenderer
{
public:
    /**
     * Initializes this window with the given parameters.
     *
     * On init failure, prints the error and aborts.
     */
    SDLRenderer(SDL_Window* window, const char* name);

    ~SDLRenderer();

    /**
     * Returns a pointer to the underlying renderer.
     */
    SDL_Renderer* get();

private:
    SDL_Renderer* renderer;
};

} // namespace AM
