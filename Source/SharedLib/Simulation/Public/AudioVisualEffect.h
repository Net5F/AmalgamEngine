#pragma once

#include "GraphicID.h"
#include "GraphicRef.h"
#include "Vector3.h"
#include "entt/fwd.hpp"
#include <vector>
#include <span>

namespace AM
{

/**
 * Describes an audio/visual effect that should occur when interacting with an 
 * item, casting a spell, etc.
 *
 * AV effects consist of "movement phases" (e.g. "move from self to target, 
 * then follow the target for 5 seconds"). Each phase's audio/visual elements 
 * follow the movement until it's completed, then the next phase is started.
 * 
 * To the server, AV effects are just data that it must keep track of along 
 * with the rest of an item/spell/etc definition. It doesn't do any processing 
 * on them.
 */
struct AudioVisualEffect {
    /**
     * Note: Some contexts may not have access to every movement type, e.g.
     *       item interactions not having a target. In such cases, it's up to 
     *       the bindings to restrict those types, since the handlers won't 
     *       be able to do anything with them.
     * Note: If this is updated, be sure to update the associated array in 
     *       requiresTarget().
     */
    enum MovementType : Uint8 {
        SelfToTarget = 0,
        TargetToSelf,
        FollowSelf,
        FollowTarget,
        StaticSelf,
        StaticTarget,
        Count
    };

    /**
     * A single movement phase.
     */
    struct MovementPhase
    {
        MovementType movementType{MovementType::FollowSelf};
        /** If movementType is one of the moving options, this is the movement 
            speed to use, in world units. */
        Uint8 movementSpeed{};
        /** If movementType is one of the Follow or Static options, this is 
            how long the phase should last, in seconds. 0 == infinite. */
        Uint8 durationS{};
        /** The number of graphics in the graphics vector that belong to this 
            phase. */
        Uint8 graphicCount{};
        // TODO: Uint8 soundCount{};
        // TODO: SDL_Rect colorMod{}; + target?
    };

    /** The movement phases that this effect has. */
    std::vector<MovementPhase> movementPhases{};

    /** The graphics for every phase. Use the phase's graphicCount to tell 
        which graphics belong to it. */
    std::vector<GraphicID> graphics{};

    // TODO: std::vector<SoundID> sounds{};

    /** Used as a "we should never hit this" cap on the container lengths. */
    static constexpr std::size_t MAX_MOVEMENT_PHASES{SDL_MAX_UINT8};
    static constexpr std::size_t MAX_GRAPHICS{SDL_MAX_UINT8};

    /**
     * Returns true if any phase in this effect has a movementType that 
     * uses the Self entity's position.
     */
    bool movementUsesSelf() const;

    /**
     * Returns true if any phase in this effect has a movementType that 
     * uses the Target entity's position.
     */
    bool movementUsesTarget() const;

    /**
     * Returns all graphics for the given movement phase.
     */
    std::span<GraphicID> getGraphicsAtPhase(Uint8 phaseIndex);
};

template<typename S>
void serialize(S& serializer, AudioVisualEffect::MovementPhase& movementPhase)
{
    serializer.value1b(movementPhase.movementType);
    serializer.value1b(movementPhase.movementSpeed);
    serializer.value1b(movementPhase.durationS);
    serializer.value1b(movementPhase.graphicCount);
}

template<typename S>
void serialize(S& serializer, AudioVisualEffect& audioVisualEffect)
{
    serializer.container(audioVisualEffect.movementPhases,
                         AudioVisualEffect::MAX_MOVEMENT_PHASES);
    serializer.container4b(audioVisualEffect.graphics,
                           AudioVisualEffect::MAX_GRAPHICS);
}

} // End namespace AM
