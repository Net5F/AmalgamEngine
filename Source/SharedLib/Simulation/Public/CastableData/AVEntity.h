#pragma once

#include "GraphicSets.h"
#include <vector>

namespace AM
{

/**
 * Defines a client-only entity for displaying audio/visual effects.
 *
 * Since A/V entities can move in irregular ways, they don't share the 
 * MovementSystem/GraphicSystem code paths with regular entities. Instead, 
 * AVSystem handles all of their updates.
 *
 * Note: Since these entities are purely visual, they're inappropriate for 
 *       something like a spell that moves around the ground and continuously 
 *       does damage. For that, you'll need to spawn a server entity.
 */
struct AVEntity {
    enum class Behavior {
        /** Start at casterEntity, move towards targetEntity. End early if it's 
            reached. */
        MoveToEntity,
        /** Start at casterEntity, move towards targetPosition. End early if 
            it's reached. */
        MoveToPosition,
        /** Start at casterEntity, move towards targetEntity for durationS 
            seconds. */
        FollowEntityStartCaster,
        /** Start at targetEntity, move towards targetEntity for durationS 
            seconds. */
        FollowEntityStartTarget,
        /** Start at casterEntity, move towards targetPosition for durationS 
            seconds. */
        FollowDirection,
        /** Stay at targetPosition for durationS seconds. */
        StaticPosition,
        /** Stay at the current position for durationS seconds. Invalid as 
            first phase. */
        CurrentPosition
    };

    struct Phase {
        /** The graphic set to use. 
            Note: We use graphic sets instead of e.g. VisualEffect because it 
                  needs to match the direction of movement. */
        EntityGraphicSetID graphicSetID{};

        // TODO: Sounds

        /** The behavior that the entity should exhibit. */
        Behavior behavior{};

        /** How fast the entity should move. */
        float movementSpeed{};

        /** How long this phase should run before timing out.
            If -1, this phase will last until the graphic plays once. If the 
            graphic is a sprite, it will end immediately. */
        float durationS{};
    };

    /** If the first phase's behavior starts at casterEntity, this is how far 
        from the caster (towards the target) to start at. Use this to avoid 
        clipping. */
    float startDistance{0};

    /** If true, this entity will be able to move in all directions. If false, 
        it will travel along the X/Y plane, maintaining the same Z height. */
    bool canMoveVertically{true};

    /** The phases that this entity will go through before being destroyed. */
    std::vector<Phase> phases{};
};

} // namespace AM
