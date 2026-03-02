#include "SDL_Wrappers/SDLWindow.h"
#include "Log.h"
#include <SDL3/SDL_video.h>

namespace AM
{

SDLWindow::SDLWindow(const std::string& title, int w, int h, Uint32 flags)
: window{SDL_CreateWindow(title.c_str(), w, h, flags)}
{
    if (!window) {
        LOG_INFO("SDL_Window init error: %s", SDL_GetError());
        std::abort();
    }
}

SDLWindow::~SDLWindow()
{
    if (window) {
        SDL_DestroyWindow(window);
    }
}

SDL_Window* SDLWindow::get()
{
    return window;
}

} // End namespace AM
