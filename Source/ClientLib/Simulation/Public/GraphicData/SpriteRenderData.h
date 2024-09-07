#pragma once

#include "AssetCache.h"
#include <SDL_rect.h>

namespace AM
{
namespace Client
{

/**
 * Holds a sprite's rendering-related data.
 *
 * See Sprite.h for more info.
 */
struct SpriteRenderData {
    /** The relative path to the sprite sheet image file that holds this
        sprite. Used for passing the sprite to our UI library, which has its
        own texture cache. */
    std::string spriteSheetRelPath{};

    /** The texture that contains this sprite. */
    std::shared_ptr<SDL_Texture> texture{};

    /** This sprite's actual-space UV position and size within its texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};

    /** The actual-space point within the sprite where the "stage" starts.
        The "stage" is the coordinate space that we overlay onto the sprite 
        image. */
    SDL_Point stageOrigin{0, 0};
};

} // End namespace Client
} // End namespace AM
