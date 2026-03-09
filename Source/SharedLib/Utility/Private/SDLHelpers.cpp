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

SDL_FColor SDLHelpers::colorToFColor(const SDL_Color& color)
{
    return {color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f};
}

void SDLHelpers::renderThickLine(SDL_Renderer* renderer, const SDL_FPoint& min,
                                 const SDL_FPoint& max, float thickness,
                                 const SDL_Color& color)
{
    // Calc the line's length.
    SDL_FPoint diff{(max.x - min.x), (max.y - min.y)};
    float length{std::sqrt((diff.x * diff.x) + (diff.y * diff.y))};
    if (length == 0) {
        LOG_ERROR("Failed to draw line: length is 0.");
        return;
    }

    // Calc a normalized vector perpendicular to the line, with a magnitude
    // of half the desired line thickness.
    // Note: If we ever end up needing to do 2D vector normalization and such
    //       again, make a Vector2 class to handle it.
    const float halfThickness{thickness / 2.f};
    SDL_FPoint perpVector{(-diff.y / length * halfThickness),
                          (diff.x / length * halfThickness)};

    // The desired thick line is a rect. Calc the verts of the rect, using the
    // perpendicular vector to step from the given line to the rect's corners.
    SDL_FColor fColor{colorToFColor(color)};
    std::array<SDL_Vertex, 4> verts{
        SDL_Vertex{{min.x + perpVector.x, min.y + perpVector.y}, fColor},
        {{min.x - perpVector.x, min.y - perpVector.y}, fColor},
        {{max.x + perpVector.x, max.y + perpVector.y}, fColor},
        {{max.x + perpVector.x, max.y + perpVector.y}, fColor}};

    // Render the rect.
    std::array<int, 6> indices{0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(renderer, nullptr, verts.data(),
                       static_cast<int>(verts.size()), indices.data(),
                       static_cast<int>(indices.size()));
}

void SDLHelpers::renderHexagon(SDL_Renderer* renderer, const SDL_FPoint& center,
                               float radius, const SDL_Color& color)
{
    // sqrt(3)/2
    const float SQRT3_2{0.8660254f};
    // Precomputed unit hexagon
    const std::array<SDL_FPoint, 6> UNIT_HEXAGON{
        SDL_FPoint{1.0f, 0.0f}, {0.5f, SQRT3_2},   {-0.5f, SQRT3_2},
        {-1.0f, 0.0f},          {-0.5f, -SQRT3_2}, {0.5f, -SQRT3_2}};

    // Calculate our verts from the unit hexagon and given parameters.
    SDL_FColor fColor{colorToFColor(color)};
    std::array<SDL_Vertex, 7> verts{};
    for (int i{0}; i < UNIT_HEXAGON.size(); ++i) {
        verts[i].position.x = center.x + (UNIT_HEXAGON[i].x * radius);
        verts[i].position.y = center.y + (UNIT_HEXAGON[i].y * radius);
        verts[i].color = fColor;
    }

    // Set up the triangle fan indices.
    std::array<int, 18> indices{};
    for (int i{0}; i < UNIT_HEXAGON.size(); ++i) {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = (i + 1) % 6 + 1;
    }

    // Render the hexagon.
    SDL_RenderGeometry(renderer, nullptr, verts.data(),
                       static_cast<int>(verts.size()), indices.data(),
                       static_cast<int>(indices.size()));
}

bool SDLHelpers::hasPositiveArea(const SDL_FRect& rect)
{
    return (rect.w > 0.0f) && (rect.h > 0.0f);
}

} // namespace AM
