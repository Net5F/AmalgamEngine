#pragma once

namespace AM
{
struct AVSequenceInstance;
namespace Client
{
class World;

/**
 * Maintains World::avSequences. Updates the sequences, and deletes them 
 * when they finish their final phase.
 */
class AVSequenceSystem
{
public:
    AVSequenceSystem(World& inWorld);

    /**
     * Update every sequence, and delete any that are finished.
     */
    void updateSequences();

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
     * Attempts to move the given sequence instance based on the given sequence 
     * definition, and returns an appropriate result.
     */
    MoveResult moveSequence(AVSequenceInstance& instance);

    World& world;
};

} // namespace Client
} // namespace AM
