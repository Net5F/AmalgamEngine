#pragma once

#include "GraphicID.h"

namespace AM
{

/**
 * Defines a single visual effect.
 *
 * Typically, this will be attached to an entity to display a temporary graphic, 
 * e.g. showing a heal graphic when an entity uses a health item.
 *
 * When the graphic is done playing, this effect is automatically destroyed.
 */
struct VisualEffect {
    /** The graphic to display. */
    GraphicID graphicID{};

    enum class LoopMode {
        PlayOnce,
        Loop
    };

    /** If the graphic is an animation, this determines whether to play the 
        animation once or to loop it. If the graphic is a sprite, this will 
        be ignored (sprites are always treated as looping). */
    LoopMode loopMode{};

    /** If loopMode == Loop, this is how long to loop for. */
    float loopTime{};
};

} // namespace AM
