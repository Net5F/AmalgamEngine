#include "AudioVisualEffect.h"
#include "AMAssert.h"
#include <array>

namespace AM
{

bool AudioVisualEffect::movementUsesSelf() const
{
    /** Holds a value for each MovementType (indexed to match) that says 
        whether it requires a self or not. */
    static constexpr std::array<bool, MovementType::Count> requiresSelfArr{
        true,  // SelfToTarget
        true,  // TargetToSelf
        true,  // FollowSelf
        false, // FollowTarget
        true,  // StaticSelf
        false  // StaticTarget
    };

    // For each phase's movement type, check if it requires a target.
    for (const MovementPhase& phase : movementPhases) {
        if (requiresSelfArr.at(phase.movementType)) {
            return true;
        }
    }

    return false;
}

bool AudioVisualEffect::movementUsesTarget() const
{
    /** Holds a value for each MovementType (indexed to match) that says 
        whether it requires a target or not. */
    static constexpr std::array<bool, MovementType::Count> requiresTargetArr{
        true,  // SelfToTarget
        true,  // TargetToSelf
        false, // FollowSelf
        true,  // FollowTarget
        false, // StaticSelf
        true   // StaticTarget
    };

    // For each phase's movement type, check if it requires a target.
    for (const MovementPhase& phase : movementPhases) {
        if (requiresTargetArr.at(phase.movementType)) {
            return true;
        }
    }

    return false;
}

std::span<GraphicID> AudioVisualEffect::getGraphicsAtPhase(Uint8 phaseIndex)
{
    AM_ASSERT(phaseIndex < movementPhases.size(), "Invalid phase index.");

    // Count to find the index of the phase's first graphic.
    int startIndex{0};
    for (Uint8 i{0}; i < (phaseIndex - 1); ++i) {
        startIndex += static_cast<int>(movementPhases.at(i).graphicCount);
    }

    MovementPhase phase{movementPhases.at(phaseIndex)};
    auto startIt{graphics.begin() + startIndex};
    return std::span<GraphicID>(startIt, startIt + phase.graphicCount);
}

} // End namespace AM
