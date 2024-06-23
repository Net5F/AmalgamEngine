#pragma once

#include <SDL_rect.h>

namespace AM
{
struct Camera;
struct Sprite;
struct Position;
struct TilePosition;
struct TileOffset;

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
    static SDL_FRect entityToScreenExtent(const Position& position,
                                          const SpriteRenderData& renderData,
                                          const Camera& camera);

    /**
     * Calculates where a tile should be drawn on screen.
     *
     * Returns a final screen space extent for the given tile position, camera,
     * and sprite.
     */
    static SDL_FRect tileToScreenExtent(const TilePosition& tilePosition,
                                        const TileOffset& tileOffset,
                                        const SpriteRenderData& renderData,
                                        const Camera& camera);
};

} // End namespace Client
} // End namespace AM
