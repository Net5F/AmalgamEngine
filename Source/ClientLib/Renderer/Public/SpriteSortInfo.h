#pragma once

#include "Sprite.h"
#include "TileLayers.h"
#include "WorldObjectID.h"
#include <SDL_rect.h>
#include <vector>
#include <variant>

namespace AM
{
namespace Client
{
/**
 * Used for storing information relevant to sorting a sprite.
 */
struct SpriteSortInfo {
    //-------------------------------------------------------------------------
    // Sprite data
    //-------------------------------------------------------------------------
    /** The sprite that is associated with this render information. */
    const Sprite* sprite;

    /** The tile layer or entity that the sprite comes from, or std::monostate 
        if this is a full phantom (not replacing an existing layer).
        Used when we pass the sorted sprites to the UI's locator. */
    WorldObjectID spriteOwnerID;

    /** The world-space bounding box that has been calculated for this sprite
        in the current frame, based on the associated entity's lerped
        position.
        Used during topological sorting. */
    BoundingBox worldBounds{0, 0, 0, 0, 0, 0};

    /** The screen extent that has been calculated for this sprite in the
        current frame, based on the associated entity's lerped position.
        Used during rendering and passed to the UI's locator. */
    SDL_Rect screenExtent{};

    /** If non-default, the UI wants us to multiply this sprite's color and 
        transparency by these values. */
    SDL_Color colorMod{255, 255, 255, 255};

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
