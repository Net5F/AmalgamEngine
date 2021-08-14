#pragma once

#include "ScreenPoint.h"
#include "Position.h"
#include "TileIndex.h"
#include <SDL2/SDL_rect.h>

namespace AM
{
class Camera;
namespace Client
{
class Sprite;

/**
 * Client-specific static functions for transforming between world and screen
 * space.
 */
class ClientTransforms
{
public:
    /**
     * Returns a final screen space extent for the given position, camera,
     * and sprite.
     */
    static SDL_Rect worldToScreenExtent(const Position& position,
                                        const Sprite& sprite,
                                        const Camera& camera);

    /**
     * Returns a final screen space extent for the given tile index, camera,
     * and sprite.
     *
     * Used as an alternative to worldToScreenExtent() since tile sprites have
     * some extra offsets applied.
     */
    static SDL_Rect tileToScreenExtent(const TileIndex& index,
                                        const Sprite& sprite,
                                       const Camera& camera);
};

} // End namespace Client
} // End namespace AM
