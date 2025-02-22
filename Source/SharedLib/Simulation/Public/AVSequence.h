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
 * Describes an audio/visual sequence that should occur when interacting with 
 * an item, casting a spell, etc.
 *
 * AV sequences consist of "movement phases" (e.g. "move from self to target, 
 * then follow the target for 5 seconds"). Each phase's audio/visual elements 
 * follow the movement until it's completed, then the next phase is started.
 */
struct AVSequence {
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

    // TODO: entity graphics to play (one shots)
    //       one for self, one for target? or vector?

    /**
     * Adds the given movement phase to the end of the vector.
     */
    void addMovementPhase(MovementType movementType, Uint8 movementSpeed,
                          Uint8 durationS);

    /**
     * Adds the given graphic to the last phase that's currently in 
     * movementPhases.
     * Errors in debug if movementPhases is empty(), does nothing in release.
     */
    void addGraphic(GraphicID graphicID);

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

    /** Used as a "we should never hit this" cap on the container lengths. */
    static constexpr std::size_t MAX_MOVEMENT_PHASES{SDL_MAX_UINT8};
    static constexpr std::size_t MAX_GRAPHICS{SDL_MAX_UINT8};

private:
    /**
     * Returns the index of the given phase's first graphic.
     */
    std::size_t getStartIndexForPhaseGraphics(Uint8 phaseIndex);
};

template<typename S>
void serialize(S& serializer, AVSequence::MovementPhase& movementPhase)
{
    serializer.value1b(movementPhase.movementType);
    serializer.value1b(movementPhase.movementSpeed);
    serializer.value1b(movementPhase.durationS);
    serializer.value1b(movementPhase.graphicCount);
}

template<typename S>
void serialize(S& serializer, AVSequence& audioVisualSequence)
{
    serializer.container(audioVisualSequence.movementPhases,
                         AVSequence::MAX_MOVEMENT_PHASES);
    serializer.container4b(audioVisualSequence.graphics,
                           AVSequence::MAX_GRAPHICS);
}

} // End namespace AM
