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
     * Returns true if the given rect contains the given point.
     *
     * Note: If the point is exactly on the edge of the rect, it still counts
     *       as being inside.
     *
     * A replacement for SDL_PointInRect(), which for some reason is not
     * inclusive of points on the far edges.
     */
    static bool pointInRect(const SDL_Point& point, const SDL_Rect& rect);

    /**
     * Returns true if rectB fully contains rectA.
     *
     * Note: If part of rectA is exactly on the edge of rectB, that part still
     *       counts as being inside.
     */
    static bool rectInRect(const SDL_Rect& rectA, const SDL_Rect& rectB);

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
