#pragma once

#include "VisualEffect.h"
#include <vector>
#include <variant>

namespace AM
{

/**
 * Tracks all audio/visual effects that are currently active on an entity.
 */
struct AVEffects {
    using AVEffect = std::variant<VisualEffect>;

    /** This entity's currently active effects. */
    std::vector<AVEffect> avEffects{};
};

} // namespace AM
