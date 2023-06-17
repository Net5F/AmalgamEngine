#include "SDLHelpers.h"
#include "Log.h"
#include <cmath>

namespace AM
{
bool SDLHelpers::pointInRect(const SDL_Point& point, const SDL_Rect& rect)
{
    // Test if the point is within all 4 sides of the rect.
    if ((point.x >= rect.x) && (point.x <= (rect.x + rect.w))
        && (point.y >= rect.y) && (point.y <= (rect.y + rect.h))) {
        return true;
    }
    else {
        return false;
    }
}

bool SDLHelpers::rectInRect(const SDL_Rect& rectA, const SDL_Rect& rectB)
{
    // Test if 2 diagonal corners of rectA are within rectB
    SDL_Point topLeft{rectA.x, rectA.y};
    SDL_Point bottomRight{(rectA.x + rectA.w), (rectA.y + rectA.h)};
    return (pointInRect(topLeft, rectB) && pointInRect(bottomRight, rectB));
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
