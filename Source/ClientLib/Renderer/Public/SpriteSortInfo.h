#pragma once

#include "Sprite.h"
#include <vector>

namespace AM
{
namespace Client
{
/**
 * Used for storing information relevant to sorting and rendering a sprite
 * on a given frame.
 */
struct SpriteSortInfo {
    /** The sprite that is associated with this render information. */
    const Sprite* sprite;

    //-------------------------------------------------------------------------
    // Render data
    //-------------------------------------------------------------------------
    /** The world-space bounding box that has been calculated for this sprite
        in the current frame, based on the associated entity's lerped
        position.
        Used during topological sorting. */
    BoundingBox worldBounds{0, 0, 0, 0, 0, 0};

    /** The screen extent that has been calculated for this sprite in the
        current frame, based on the associated entity's lerped position.
        Used during rendering. */
    SDL_Rect screenExtent{};

    //-------------------------------------------------------------------------
    // Topological sort data
    //-------------------------------------------------------------------------
    /** The depth value of the sprite in the current frame.
        Higher value means further in front. */
    int depthValue{0};

    /** The sprites that are behind this sprite in the current frame.
        Necessary for our topological sort. */
    std::vector<SpriteSortInfo*> spritesBehind{};

    /** True if this sprite has been visited on the current topological
        sort pass. */
    bool visited{false};
};

} // End namespace Client
} // End namespace AM
