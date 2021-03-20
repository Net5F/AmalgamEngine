#pragma once

#include "Sprite.h"
#include <vector>

namespace AM
{
/**
 * Used for storing information relevant to sorting and rendering a sprite
 * on a given frame.
 */
struct SpriteRenderInfo {
    /** The sprite that is associated with this render information. */
    Sprite* sprite;

    /** The screen extent that has been calculated for this sprite in the
        current frame. */
    SDL2pp::Rect screenExtent{};

    /** The depth value of the sprite in the current frame.
        Higher value means further in front. */
    int depthValue{0};

    /** The sprites that are behind this sprite in the current frame.
        Necessary for our topological sort. */
    std::vector<SpriteRenderInfo*> spritesBehind{};

    /** True if this sprite has been visited on the current topological
        sort pass. */
    bool visited{false};
};

} // End namespace AM
