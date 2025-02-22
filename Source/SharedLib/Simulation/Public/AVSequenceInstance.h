#pragma once

#include "AVSequence.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "entt/fwd.hpp"

namespace AM
{

/**
 * An in-world instance of an AVSequence.
 */
struct AVSequenceInstance {
    /** A copy of the sequence that this instance is based on.
        Note: We need to copy the sequence instead of referencing it, since the
              owner may be updated at any time. */
    AVSequence sequence{};

    /** The entity that initiated this sequence. May be null if not relevant. */
    entt::entity selfEntity{};

    /** The target of this sequence. May be null if not relevnt. */
    entt::entity targetEntity{};

    /** This sequence's current position. */
    Position position{};

    /** This sequence's previous position (used for lerping in the renderer). */
    PreviousPosition prevPosition{};

    /** The latest position of the initiator (if relevant).
        We track this so that, if the entity disappears (dies, logs off, etc),
        we can still finish moving this sequence. */
    Position selfPosition{};

    /** The latest position of our target (if relevant).
        We track this so that, if the entity disappears (dies, logs off, etc),
        we can still finish moving this sequence. */
    Position targetPosition{};

    /** The index of the currently active phase within 
        sequence.movementPhases. */
    Uint8 currentPhaseIndex{0};

    /** A timestamp of when the current phase was started.
        Also used for any animation graphics. */
    double phaseStartTime{0};

    /** If true, a new phase has begun and phaseStartTime needs to be reset when
        the phase is first rendered. */
    bool setStartTime{true};
};

} // End namespace AM
