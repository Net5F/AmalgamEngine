#pragma once

#include "ScreenPoint.h"
#include "Position.h"
#include "TilePosition.h"
#include <SDL_rect.h>

namespace AM
{
struct Camera;
struct Sprite;
namespace Client
{
struct SpriteRenderData;

/**
 * Client-specific static functions for transforming between world and screen
 * space.
 */
class ClientTransforms
{
public:
    /**
     * Calculates where an entity should be drawn on screen.
     *
     * Returns a final screen space extent for the given position, camera,
     * and sprite.
     */
    static SDL_Rect entityToScreenExtent(const Position& position,
                                         const SpriteRenderData& renderData,
                                         const Camera& camera);

    /**
     * Calculates where a tile should be drawn on screen.
     *
     * Returns a final screen space extent for the given tile position, camera,
     * and sprite.
     */
    static SDL_Rect tileToScreenExtent(const TilePosition& position,
                                       const SpriteRenderData& renderData,
                                       const Camera& camera);
};

} // End namespace Client
} // End namespace AM
