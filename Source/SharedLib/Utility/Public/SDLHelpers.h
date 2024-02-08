#pragma once

#include <SDL_rect.h>

namespace AM
{
struct Ray;

/**
 * Static functions for working with SDL types.
 */
class SDLHelpers
{
public:
    /**
     * Converts the given SDL_Point to an SDL_FPoint.
     */
    static SDL_FPoint pointToFPoint(const SDL_Point& point);

    /**
     * Converts the given SDL_FPoint to an SDL_Point, rounding each value
     * towards zero.
     */
    static SDL_Point truncateFPoint(const SDL_FPoint& point);

    /**
     * Converts the given SDL_FPoint to an SDL_Point, rounding each value to
     * the nearest whole number.
     */
    static SDL_Point roundFPoint(const SDL_FPoint& point);

    /**
     * Converts the given SDL_Rect to an SDL_FRect.
     */
    static SDL_FRect rectToFRect(const SDL_Rect& rect);

    /**
     * Converts the given SDL_FRect to an SDL_Rect, rounding each value towards
     * zero.
     */
    static SDL_Rect truncateFRect(const SDL_FRect& rect);

    /**
     * Converts the given SDL_FRect to an SDL_Rect, rounding each value to
     * the nearest whole number.
     */
    static SDL_Rect roundFRect(const SDL_FRect& rect);
};

} // namespace AM
