#pragma once

#include "VisualEffect.h"
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
    using AVEffect = std::variant<VisualEffect, /* TODO: AudioEffect */>;

    /** This entity's currently active effects. */
    std::vector<AVEffect> avEffects{};
};

} // namespace Client
} // namespace AM
