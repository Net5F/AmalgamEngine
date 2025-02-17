#pragma once

#include "AudioVisualEffect.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "entt/fwd.hpp"

namespace AM
{

/**
 * An in-world instance of an AudioVisualEffect.
 */
struct AudioVisualEffectInstance {
    /** A copy of the effect that this instance is based on.
        Note: We need to copy the effect instead of referencing it, since the
              owner may be updated at any time. */
    AudioVisualEffect effect{};

    /** The entity that initiated this effect. May be null if not relevant. */
    entt::entity selfEntity{};

    /** The target of this effect. May be null if not relevnt. */
    entt::entity targetEntity{};

    /** This effect's current position. */
    Position position{};

    /** This effect's previous position (used for lerping in the renderer). */
    PreviousPosition prevPosition{};

    /** The latest position of the initiator (if relevant).
        We track this so that, if the entity disappears (dies, logs off, etc),
        we can still finish moving this effect. */
    Position selfPosition{};

    /** The latest position of our target (if relevant).
        We track this so that, if the entity disappears (dies, logs off, etc),
        we can still finish moving this effect. */
    Position targetPosition{};

    /** The index of the currently active phase within effect.movementPhases. */
    Uint8 currentPhaseIndex{0};

    /** A timestamp of when the current phase was started.
        Also used for any animation graphics. */
    double phaseStartTime{0};

    /** If true, a new phase has begun and phaseStartTime needs to be reset when
        the phase is first rendered. */
    bool setStartTime{true};
};

} // End namespace AM
