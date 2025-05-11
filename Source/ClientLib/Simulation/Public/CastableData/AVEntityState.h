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
 * Defines a client-only entity for displaying audio/visual effects.
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
};

} // namespace Client
} // namespace AM
