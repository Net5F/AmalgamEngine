#include "SDLHelpers.h"
#include "Ray.h"
#include "Log.h"
#include <SDL_render.h>
#include <SDL_image.h>
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

    // Get the texture's dimensions.
    int width{};
    int height{};
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);

    // Copy the texture into a surface.
    SDL_Surface* surface{SDL_CreateRGBSurface(
        0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF)};
    SDL_RenderReadPixels(renderer, NULL, surface->format->format,
                         surface->pixels, surface->pitch);

    // Save the texture to a file.
    int result{IMG_SavePNG(surface, filePath.c_str())};
    SDL_FreeSurface(surface);

    SDL_SetRenderTarget(renderer, oldRenderTarget);
    return (result == 0);
}

} // namespace AM
