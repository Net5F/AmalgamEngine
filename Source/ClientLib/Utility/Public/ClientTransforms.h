#pragma once

#include <SDL_rect.h>

namespace AM
{
struct Camera;
struct Sprite;
struct Position;
struct Vector3;
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
     * @return A final screen space extent for the entity.
     */
    static SDL_FRect entityToScreenExtent(const Position& position,
                                          const Vector3& idleSouthBottomCenter,
                                          const Vector3& alignmentOffset,
                                          const SpriteRenderData& renderData,
                                          const Camera& camera);

    /**
     * Calculates where a tile should be drawn on screen.
     *
     * @return A final screen space extent for the tile.
     */
    static SDL_FRect tileToScreenExtent(const TilePosition& tilePosition,
                                        const TileOffset& tileOffset,
                                        const SpriteRenderData& renderData,
                                        const Camera& camera);
};

} // End namespace Client
} // End namespace AM
