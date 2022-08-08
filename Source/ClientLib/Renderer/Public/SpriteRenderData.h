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
    /** The texture that contains this sprite. */
    TextureHandle texture{};

    /** UV position and size in texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};

    /** How much this sprite should be offset in the Y direction to line up
        with its tile. Used to support tall sprites for the iso depth effect. */
    int yOffset{0};
};

} // End namespace Client
} // End namespace AM
