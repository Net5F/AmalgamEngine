#include "SDLHelpers.h"
#include "Ray.h"
#include "Log.h"
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

} // namespace AM
