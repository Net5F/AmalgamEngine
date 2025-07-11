#pragma once

#include "AVEntity.h"
#include "Vector3.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <functional>

namespace AM
{
namespace Client
{

/**
 * An instance of a client-only entity for displaying audio/visual effects.
 */
struct AVEntityState {
    /** The definition that this entity is an instance of.
        Note: This should never be nullptr. */
    std::reference_wrapper<const AVEntity> avEntity;

    /** The target entity, if one was provided. */
    entt::entity targetEntity{entt::null};

    /** The target position, if one was provided. */
    Vector3 targetPosition{0, 0, 0};

    /** The index within phases of the current phase. */
    std::size_t currentPhaseIndex{0};

    /** Used to track when the phase is incremented, so we can reset timers 
        on the first tick of the new phase. */
    bool phaseWasIncremented{false};

    //-------------------------------------------------------------------------
    // Timing state (managed by WorldSpriteSorter)
    //-------------------------------------------------------------------------
    /** A timestamp of when the current phase was started. */
    double phaseStartTime{0};

    /** If true, a new phase has begun and startTime needs to be reset when 
        its first frame is rendered. */
    bool setStartTime{true};
};

} // namespace Client
} // namespace AM
