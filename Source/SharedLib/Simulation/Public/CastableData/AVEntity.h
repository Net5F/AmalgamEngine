#pragma once

#include "GraphicSets.h"
#include "entt/fwd.hpp"
#include <vector>

namespace AM
{

/**
 * Defines a client-only entity for displaying audio/visual effects.
 */
struct AVEntity {
    enum class AVEntityBehavior {
        /** Move towards targetEntity. End early if it's reached. */
        MoveToEntity,
        /** Move towards targetPosition. End early if it's reached. */
        MoveToPosition,
        /** Move towards targetEntity for durationS seconds. */
        FollowEntity
    };

    struct Phase {
        /** The graphic set to use. */
        EntityGraphicSet graphicSet{};

        // TODO: Sounds

        /** The behavior that the entity should exhibit. */
        AVEntityBehavior behavior{};

        /** How fast the entity should move. */
        float movementSpeed{};

        /** How long this phase should run before timing out. */
        float durationS{};
    };

    /** The target entity, if one was provided. */
    entt::entity targetEntity{};

    /** The target position, if one was provided. */
    Vector3 targetPosition{};

    /** The phases that this entity will go through before being destroyed. */
    std::vector<Phase> phases{};
};

} // namespace AM
