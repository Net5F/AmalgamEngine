#include "AudioVisualEffectSystem.h"
#include "AudioVisualEffectInstance.h"
#include "World.h"
#include "Position.h"
#include "Timer.h"
#include "Log.h"

namespace AM
{
namespace Client
{
AudioVisualEffectSystem::AudioVisualEffectSystem(World& inWorld)
: world{inWorld}
{
}

void AudioVisualEffectSystem::updateEffects()
{
    // Iterate every phase, updating and erasing as necessary.
    for (auto it{world.audioVisualEffects.begin()};
         it != world.audioVisualEffects.end();) {
        // If this effect has completed, kill it and continue.
        AudioVisualEffectInstance& instance{it->second};
        if (instance.currentPhaseIndex
            >= instance.effect.movementPhases.size()) {
            it = world.audioVisualEffects.erase(it);
            continue;
        }

        // Move the effect, if necessary. If the movement failed, kill this 
        // effect and continue.
        MoveResult moveResult{moveEffect(instance)};
        if (moveResult == MoveResult::Failure) {
            it = world.audioVisualEffects.erase(it);
            continue;
        }

        // If the current phase completed, increment to the next.
        if (moveResult == MoveResult::PhaseCompleted) {
            instance.currentPhaseIndex++;
            instance.setStartTime = true;
        }

        it++;
    }
}

AudioVisualEffectSystem::MoveResult
    AudioVisualEffectSystem::moveEffect(AudioVisualEffectInstance& instance)
{
    using MovementType = AudioVisualEffect::MovementType;

    auto& currentPhase{
        instance.effect.movementPhases.at(instance.currentPhaseIndex)};
    MovementType movementType{currentPhase.movementType};

    // If this phase's movement uses the initiating entity's current position, 
    // try to update it.
    if ((currentPhase.movementType == MovementType::FollowSelf)
        || (currentPhase.movementType == MovementType::TargetToSelf)) {
        if (Position
            * selfPos{
                world.registry.try_get<Position>(instance.selfEntity)}) {
            instance.selfPosition = *selfPos;
        }
        else {
            // The initiating entity is gone (died, disconnected, etc). That's 
            // fine, we'll just keep using the last saved position.
        }
    }

    // If this phase's movement uses the target entity's current position, try 
    // to update it.
    if ((currentPhase.movementType == MovementType::FollowTarget)
        || (currentPhase.movementType == MovementType::SelfToTarget)) {
        if (Position
            * targetPos{
                world.registry.try_get<Position>(instance.targetEntity)}) {
            instance.targetPosition = *targetPos;
        }
        else {
            // The target entity is gone (died, disconnected, etc). That's 
            // fine, we'll just keep using the last saved position.
        }
    }

    // If we need to move, do so.
    if ((currentPhase.movementType == MovementType::TargetToSelf)
        || (currentPhase.movementType == MovementType::FollowSelf)) {
        instance.position = instance.position.moveTowards(
            instance.selfPosition,
            static_cast<float>(currentPhase.movementSpeed));
    }
    else if ((currentPhase.movementType == MovementType::SelfToTarget)
             || (currentPhase.movementType == MovementType::FollowTarget)) {
        instance.position = instance.position.moveTowards(
            instance.targetPosition,
            static_cast<float>(currentPhase.movementSpeed));
    }

    // Determine if the movement phase has been completed.
    double elapsedTime{Timer::getGlobalTime() - instance.phaseStartTime};
    bool phaseCompleted{false};
    if ((currentPhase.movementType == MovementType::StaticSelf)
        || (currentPhase.movementType == MovementType::StaticTarget)
        || (currentPhase.movementType == MovementType::FollowSelf)
        || (currentPhase.movementType == MovementType::FollowTarget)) {
        // Time-based movement. Check if enough time has passed.
        if ((currentPhase.durationS != 0)
            && elapsedTime >= static_cast<double>(currentPhase.durationS)) {
            phaseCompleted = true;
        }
    }
    else if ((currentPhase.movementType == MovementType::SelfToTarget)
             || (currentPhase.movementType == MovementType::TargetToSelf)) {
        // Position-based movement. Check if we've reached the end.
        if (((currentPhase.movementType == MovementType::SelfToTarget)
             && (instance.position == instance.targetPosition))
            || ((currentPhase.movementType == MovementType::TargetToSelf)
                && (instance.position == instance.selfPosition))) {
            phaseCompleted = true;
        }

        // If we haven't reached the end, check if we've timed out.
        // Note: There's no infinite duration for this movement. Use Follow + 
        //       Self instead.
        if (!phaseCompleted
            && (elapsedTime >= static_cast<double>(currentPhase.durationS))) {
            phaseCompleted = true;
        }
    }

    // Return an appropriate result.
    if (phaseCompleted) {
        return MoveResult::PhaseCompleted;
    }
    else {
        return MoveResult::Success;
    }
}

} // namespace Client
} // namespace AM
