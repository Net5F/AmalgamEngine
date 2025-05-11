#pragma once

#include "VisualEffect.h"
#include <functional>

namespace AM
{
namespace Client
{

/**
 * Defines a single visual effect.
 *
 * Typically, this will be attached to an entity to display a temporary graphic, 
 * e.g. showing a heal graphic when an entity uses a health item.
 *
 * When the graphic is done playing, this effect is automatically destroyed.
 */
struct VisualEffectState {
    /** The definition that this is an instance of. */
    std::reference_wrapper<const VisualEffect> visualEffect;

    //-------------------------------------------------------------------------
    // Timing state (managed by WorldSpriteSorter)
    //-------------------------------------------------------------------------
    /** A timestamp of when this effect was started.
        If 0, this effect has not yet started playing. */
    double startTime{0};
};

} // namespace Client
} // namespace AM
