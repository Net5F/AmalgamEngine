#include "SDLHelpers.h"
#include "Ray.h"
#include "Log.h"
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_video.h>
#include <cmath>

namespace AM
{
SDL_FPoint SDLHelpers::pointToFPoint(const SDL_Point& point)
{
    return {static_cast<float>(point.x), static_cast<float>(point.y)};
}

SDL_Point SDLHelpers::truncateFPoint(const SDL_FPoint& point)
{
    return {static_cast<int>(point.x), static_cast<int>(point.y)};
}

SDL_Point SDLHelpers::roundFPoint(const SDL_FPoint& point)
{
    return {static_cast<int>(std::round(point.x)),
            static_cast<int>(std::round(point.y))};
}

SDL_FRect SDLHelpers::rectToFRect(const SDL_Rect& rect)
{
    return {static_cast<float>(rect.x), static_cast<float>(rect.y),
            static_cast<float>(rect.w), static_cast<float>(rect.h)};
}

SDL_Rect SDLHelpers::truncateFRect(const SDL_FRect& rect)
{
    return {static_cast<int>(rect.x), static_cast<int>(rect.y),
            static_cast<int>(rect.w), static_cast<int>(rect.h)};
}

SDL_Rect SDLHelpers::roundFRect(const SDL_FRect& rect)
{
    return {static_cast<int>(std::round(rect.x)),
            static_cast<int>(std::round(rect.y)),
            static_cast<int>(std::round(rect.w)),
            static_cast<int>(std::round(rect.h))};
}

bool SDLHelpers::savePng(const std::string& filePath, SDL_Renderer* renderer,
                         SDL_Texture* texture)
{
    // Save the old render target and set the given texture as our new target.
    SDL_Texture* oldRenderTarget{SDL_GetRenderTarget(renderer)};
    SDL_SetRenderTarget(renderer, texture);

    // Copy the texture into a surface.
    SDL_Surface* surface{SDL_RenderReadPixels(renderer, nullptr)};

    // Save the surface to a file.
    int result{IMG_SavePNG(surface, filePath.c_str())};
    SDL_DestroySurface(surface);

    SDL_SetRenderTarget(renderer, oldRenderTarget);
    return (result == 0);
}

void SDLHelpers::setWindowFullscreen(SDL_Window* window, Uint32 newMode)
{
    if (!SDL_SetWindowFullscreen(window, newMode)) {
        LOG_INFO("Failed to set fullscreen mode: %s", SDL_GetError());
    }
}

} // namespace AM
