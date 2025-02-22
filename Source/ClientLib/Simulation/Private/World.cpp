#include "World.h"
#include "EnttGroups.h"

namespace AM
{
namespace Client
{
World::World(GraphicData& graphicData)
: registry{}
, avSequences{}
, itemData{}
, playerEntity{entt::null}
, entityLocator{registry}
, collisionLocator{}
, tileMap{graphicData, collisionLocator}
, avSequenceIDPool{IDPool::ReservationStrategy::ReuseLowest, 32}
{
    // Initialize our entt groups, before anyone tries to use them.
    EnttGroups::init(registry);

    // Reserve the null AV sequence ID.
    avSequenceIDPool.reserveID();
}

AVSequenceID
    World::addAVSequence(const AVSequence& avEffect,
                                entt::entity selfEntity,
                                entt::entity targetEntity)
{
    // If the sequence doesn't have any phases, fail.
    if (avEffect.movementPhases.empty()) {
        return NULL_AV_SEQUENCE_ID;
    }

    // If the sequence uses the Self entity and selfEntity doesn't exist, fail.
    Position* selfEntityPos{nullptr};
    if (avEffect.movementUsesSelf()) {
        selfEntityPos = registry.try_get<Position>(selfEntity);
        if (!selfEntityPos) {
            return NULL_AV_SEQUENCE_ID;
        }
    }

    // If the sequence uses the Target entity and targetEntity doesn't exist, 
    // fail.
    Position* targetEntityPos{nullptr};
    if (avEffect.movementUsesTarget()) {
        targetEntityPos = registry.try_get<Position>(targetEntity);
        if (!targetEntityPos) {
            return NULL_AV_SEQUENCE_ID;
        }
    }

    // Reserve an ID and create an empty sequence instance.
    AVSequenceID sequenceID{
        static_cast<AVSequenceID>(avSequenceIDPool.reserveID())};
    avSequences.emplace(sequenceID, AVSequenceInstance{});
    AVSequenceInstance& sequenceInstance{avSequences.at(sequenceID)};

    // Fill the instance with the sequence's data.
    sequenceInstance.sequence = avEffect;
    sequenceInstance.selfEntity = selfEntity;
    sequenceInstance.targetEntity = targetEntity;
    if (selfEntityPos) {
        sequenceInstance.selfPosition = *selfEntityPos;
    }
    if (targetEntityPos) {
        sequenceInstance.targetPosition = *targetEntityPos;
    }
    sequenceInstance.setStartTime = true;
    // Note: We don't init phaseStartTime here because its timer shouldn't 
    //       start until it's actually rendered.

    // Init the instance's position based on the movement type of the 
    // sequence's first phase.
    using MovementType = AVSequence::MovementType;
    MovementType movementType{avEffect.movementPhases.at(0).movementType};
    if ((movementType == MovementType::SelfToTarget)
        || (movementType == MovementType::FollowSelf)
        || (movementType == MovementType::StaticSelf)) {
        sequenceInstance.position = sequenceInstance.selfPosition;
    }
    else if ((movementType == MovementType::TargetToSelf)
        || (movementType == MovementType::FollowTarget)
        || (movementType == MovementType::StaticTarget)) {
        sequenceInstance.position = sequenceInstance.targetPosition;
    }
    sequenceInstance.prevPosition = sequenceInstance.position;

    return sequenceID;
}

void World::remAVSequence(AVSequenceID avEffectID)
{
    // Find the sequence in the map.
    auto avEffectIt{avSequences.find(avEffectID)};
    if (avEffectIt == avSequences.end()) {
        LOG_FATAL("Invalid ID while removing AV sequence.");
    }

    // Free the sequence's ID.
    avSequenceIDPool.freeID(avEffectID);

    // Erase the sequence.
    avSequences.erase(avEffectIt);
}

} // End namespace Client
} // End namespace AM
