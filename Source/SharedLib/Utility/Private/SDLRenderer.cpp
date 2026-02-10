#include "SDL_Wrappers/SDLRenderer.h"
#include "Log.h"
#include <SDL_render.h>

namespace AM
{

SDLRenderer::SDLRenderer(SDL_Window* window, int index, Uint32 flags)
: renderer{SDL_CreateRenderer(window, index, flags)}
{
    if (!renderer) {
        LOG_INFO("SDL_Renderer init error: %s", SDL_GetError());
        std::abort();
    }
}

SDLRenderer::~SDLRenderer()
{
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
}

SDL_Renderer* SDLRenderer::get()
{
    return renderer;
}

} // End namespace AM
