#include "SDL_Wrappers/SDLRenderer.h"
#include "Log.h"
#include <SDL3/SDL_render.h>

namespace AM
{

SDLRenderer::SDLRenderer(SDL_Window* window, const char* name)
: renderer{SDL_CreateRenderer(window, name)}
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
