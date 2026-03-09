#pragma once

#include <string>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_pixels.h>

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

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

    /**
     * Saves the given texture to a png file at the given path.
     * If the file already exists, it will be overwritten.
     *
     * @return true if successful, else false.
     */
    static bool savePng(const std::string& filePath, SDL_Renderer* renderer,
                        SDL_Texture* texture);

    /**
     * Sets the fullscreen mode of the given window to the given mode.
     *
     * On failure, prints the error.
     */
    static void setWindowFullscreen(SDL_Window* window, Uint32 newMode);

    /**
     * Converts a [0 - 255] color to a [0 - 1] float color.
     */
    static SDL_FColor colorToFColor(const SDL_Color& color);

    /**
     * Renders a thick line that spans from min to max, with the given
     * thickness and color.
     */
    static void renderThickLine(SDL_Renderer* renderer, const SDL_FPoint& min,
                                const SDL_FPoint& max, float thickness,
                                const SDL_Color& color);

    /**
     * Renders a hexagon with the given radius and color at the given point.
     */
    static void renderHexagon(SDL_Renderer* renderer, const SDL_FPoint& center,
                              float radius, const SDL_Color& color);

    /**
     * Returns true if the given rect has a positive area.
     */
    static bool hasPositiveArea(const SDL_FRect& rect);
};

} // namespace AM
