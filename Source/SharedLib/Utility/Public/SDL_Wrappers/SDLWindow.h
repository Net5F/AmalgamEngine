#pragma once

#include <SDL_stdinc.h>
#include <string>

struct SDL_Window;

namespace AM
{
/**
 * RAII wrapper for SDL_Window.
 */
class SDLWindow
{
public:
    /**
     * Initializes this window with the given parameters.
     *
     * On init failure, prints the error and aborts.
     */
    SDLWindow(const std::string& title, int x, int y, int w, int h,
              Uint32 flags);

	~SDLWindow();

    /**
     * Returns a pointer to the underlying window.
     */
    SDL_Window* get();

private:
    SDL_Window* window;
};

} // namespace AM
