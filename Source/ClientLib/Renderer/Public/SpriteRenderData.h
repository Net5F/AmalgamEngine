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
public:
    /** The relative path to the sprite sheet image file that holds this
        sprite. Used for passing the sprite to our UI library, which has its
        own texture cache. */
    std::string spriteSheetRelPath{};

    /** The texture that contains this sprite. */
    std::shared_ptr<SDL_Texture> texture{};

    /** UV position and size in texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};

    /** How much this sprite should be offset in the Y direction to line up
        with its tile. Used to support tall sprites for the iso depth effect. */
    int yOffset{0};
};

} // End namespace Client
} // End namespace AM
