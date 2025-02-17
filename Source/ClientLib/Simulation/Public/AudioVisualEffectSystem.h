#pragma once

namespace AM
{
struct AudioVisualEffectInstance;
namespace Client
{
class World;

/**
 * Maintains World::audioVisualEffects. Updates the effects, and deletes them 
 * when they finish their final phase.
 */
class AudioVisualEffectSystem
{
public:
    AudioVisualEffectSystem(World& inWorld);

    /**
     * Update every effect, and delete any that are finished.
     */
    void updateEffects();

private:
    enum class MoveResult {
        // The movement succeeded and there's further to go.
        Success,
        // The movement succeeded and the current phase is now complete.
        PhaseCompleted,
        // The movement failed.
        Failure
    };

    /** The maximum length of time that we'll let a movement phase persist 
        for, unless it's infinite (durationS == 0). */
    static constexpr double MOVEMENT_TIMEOUT_S{20};

    /**
     * Attempts to move the given effect instance based on the given effect 
     * definition, and returns an appropriate result.
     */
    MoveResult moveEffect(AudioVisualEffectInstance& instance);

    World& world;
};

} // namespace Client
} // namespace AM
