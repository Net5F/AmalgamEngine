#include "AVSequence.h"
#include "AMAssert.h"
#include <array>

namespace AM
{

void AVSequence::addMovementPhase(MovementType movementType,
                                         Uint8 movementSpeed, Uint8 durationS)
{
    if (movementPhases.size() == MAX_MOVEMENT_PHASES) {
        LOG_ERROR("Tried to add phase to full AV effect.");
        return;
    }

    movementPhases.emplace_back(movementType, movementSpeed, durationS, 0);
}

void AVSequence::addGraphic(GraphicID graphicID)
{
    if (movementPhases.empty()) {
        LOG_ERROR("Tried to add graphic to a sequence that has no phases");
        return;
    }

    graphics.push_back(graphicID);
    movementPhases.back().graphicCount++;
}

bool AVSequence::movementUsesSelf() const
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

bool AVSequence::movementUsesTarget() const
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

std::span<GraphicID> AVSequence::getGraphicsAtPhase(Uint8 phaseIndex)
{
    AM_ASSERT(phaseIndex < movementPhases.size(), "Invalid phase index.");

    MovementPhase phase{movementPhases.at(phaseIndex)};
    std::size_t startIndex{getStartIndexForPhaseGraphics(phaseIndex)};
    auto startIt{graphics.begin() + startIndex};
    return std::span<GraphicID>(startIt, startIt + phase.graphicCount);
}

std::size_t AVSequence::getStartIndexForPhaseGraphics(Uint8 phaseIndex)
{
    // Note: We only assert here because this should be handle by the caller.
    AM_ASSERT(phaseIndex < movementPhases.size(), "Invalid phase index.");

    // Count to find the index of the phase's first graphic.
    std::size_t startIndex{0};
    for (Uint8 i{0}; i < (phaseIndex - 1); ++i) {
        startIndex += movementPhases.at(i).graphicCount;
    }
    return startIndex;
}

} // End namespace AM
