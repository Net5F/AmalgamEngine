#include "SDL_Wrappers/SDL.h"
#include "Log.h"
#include <SDL.h>

namespace AM
{

SDL::SDL(Uint32 flags)
{
    if (SDL_Init(flags) != 0) {
        LOG_INFO("SDL_Init error: %s", SDL_GetError());
        std::abort();
    }
}

SDL::~SDL()
{
    SDL_Quit();
}

} // End namespace AM
