#pragma once

#include "VisualEffectState.h"
#include <vector>
#include <variant>

namespace AM
{
namespace Client
{

/**
 * Tracks all audio/visual effects that are currently active on an entity.
 */
struct AVEffects {
    /** This entity's currently active visual effects. */
    std::vector<VisualEffectState> visualEffects{};

    /** This entity's currently active audio effects. */
    //std::vector<AVEffect> avEffects{};
};

} // namespace Client
} // namespace AM
