#include "World.h"
#include "EnttGroups.h"

namespace AM
{
namespace Client
{
World::World(GraphicData& graphicData)
: registry{}
, audioVisualEffects{}
, itemData{}
, playerEntity{entt::null}
, entityLocator{registry}
, collisionLocator{}
, tileMap{graphicData, collisionLocator}
, audioVisualEffectIDPool{IDPool::ReservationStrategy::ReuseLowest, 32}
{
    // Initialize our entt groups, before anyone tries to use them.
    EnttGroups::init(registry);

    // Reserve the null AV effect ID.
    audioVisualEffectIDPool.reserveID();
}

AudioVisualEffectID
    World::addAudioVisualEffect(const AudioVisualEffect& avEffect,
                                entt::entity selfEntity,
                                entt::entity targetEntity)
{
    // If the effect doesn't have any phases, fail.
    if (avEffect.movementPhases.empty()) {
        return NULL_AUDIO_VISUAL_EFFECT_ID;
    }

    // If the effect uses the Self entity and selfEntity doesn't exist, fail.
    Position* selfEntityPos{nullptr};
    if (avEffect.movementUsesSelf()) {
        selfEntityPos = registry.try_get<Position>(selfEntity);
        if (!selfEntityPos) {
            return NULL_AUDIO_VISUAL_EFFECT_ID;
        }
    }

    // If the effect uses the Target entity and targetEntity doesn't exist, fail.
    Position* targetEntityPos{nullptr};
    if (avEffect.movementUsesTarget()) {
        targetEntityPos = registry.try_get<Position>(targetEntity);
        if (!targetEntityPos) {
            return NULL_AUDIO_VISUAL_EFFECT_ID;
        }
    }

    // Reserve an ID and create an empty effect instance.
    AudioVisualEffectID effectID{
        static_cast<AudioVisualEffectID>(audioVisualEffectIDPool.reserveID())};
    audioVisualEffects.emplace(effectID, AudioVisualEffectInstance{});
    AudioVisualEffectInstance& effectInstance{audioVisualEffects.at(effectID)};

    // Fill the instance with the effect's data.
    effectInstance.effect = avEffect;
    effectInstance.selfEntity = selfEntity;
    effectInstance.targetEntity = targetEntity;
    if (selfEntityPos) {
        effectInstance.selfPosition = *selfEntityPos;
    }
    if (targetEntityPos) {
        effectInstance.targetPosition = *targetEntityPos;
    }
    effectInstance.setStartTime = true;
    // Note: We don't init phaseStartTime here because its timer shouldn't 
    //       start until it's actually rendered.

    // Init the instance's position based on the movement type of the 
    // effect's first phase.
    using MovementType = AudioVisualEffect::MovementType;
    MovementType movementType{avEffect.movementPhases.at(0).movementType};
    if ((movementType == MovementType::SelfToTarget)
        || (movementType == MovementType::FollowSelf)
        || (movementType == MovementType::StaticSelf)) {
        effectInstance.position = effectInstance.selfPosition;
    }
    else if ((movementType == MovementType::TargetToSelf)
        || (movementType == MovementType::FollowTarget)
        || (movementType == MovementType::StaticTarget)) {
        effectInstance.position = effectInstance.targetPosition;
    }
    effectInstance.prevPosition = effectInstance.position;

    return effectID;
}

void World::remAudioVisualEffect(AudioVisualEffectID avEffectID)
{
    // Find the effect in the map.
    auto avEffectIt{audioVisualEffects.find(avEffectID)};
    if (avEffectIt == audioVisualEffects.end()) {
        LOG_FATAL("Invalid ID while removing AV effect.");
    }

    // Free the effect's ID.
    audioVisualEffectIDPool.freeID(avEffectID);

    // Erase the effect.
    audioVisualEffects.erase(avEffectIt);
}

} // End namespace Client
} // End namespace AM
