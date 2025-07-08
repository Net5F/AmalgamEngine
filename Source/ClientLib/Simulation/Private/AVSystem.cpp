#include "AVSystem.h"
#include "World.h"
#include "GraphicData.h"
#include "ClientGraphicState.h"
#include "GraphicHelpers.h"
#include "AVEffects.h"
#include "AVEntityState.h"
#include "AVEntityHelpers.h"
#include "EnttGroups.h"
#include "Timer.h"

namespace AM
{
namespace Client
{

AVSystem::AVSystem(World& inWorld, const GraphicData& inGraphicData)
: world{inWorld}
, graphicData{inGraphicData}
{
}

void AVSystem::updateAVEffectsAndEntities()
{
    updateAVEffects();

    updateAVEntities();
}

void AVSystem::updateAVEffects()
{
    auto view{world.registry.view<AVEffects>()};
    for (auto [entity, avEffects] : view.each()) {
        // Update visual effects.
        updateVisualEffects(avEffects.visualEffects);

        // Update audio effects.
    }
}

void AVSystem::updateVisualEffects(
    std::vector<VisualEffectState>& visualEffects)
{
    double currentTime{Timer::getGlobalTime()};

    for (auto it{visualEffects.begin()}; it != visualEffects.end();) {
        // If the effect hasn't been started yet, skip it.
        VisualEffectState& effectState{*it};
        if (effectState.startTime == 0) {
            it++;
            continue;
        }

        // Determine whether this effect's graphic is a sprite or animation.
        bool isAnimation{
            isAnimationID(effectState.visualEffect.get().graphicID)};

        // If this is a PlayOnce animation, check if we've reached the end.
        double endTime{};
        if (isAnimation
            && (it->visualEffect.get().loopMode
                == VisualEffect::LoopMode::PlayOnce)) {
            const Animation& animation{graphicData.getAnimation(
                toAnimationID(effectState.visualEffect.get().graphicID))};
            endTime = effectState.startTime + animation.getLengthS();
        }
        else {
            // LoopMode::Loop.
            endTime = effectState.startTime
                      + effectState.visualEffect.get().loopTime;
        }

        // If the effect is completed, destroy it.
        if (currentTime >= endTime) {
            it = visualEffects.erase(it);
        }
        else {
            it++;
        }
    }
}

void AVSystem::updateAVEntities()
{
    double currentTime{Timer::getGlobalTime()};

    // Process the current phase for each A/V entity.
    auto view{world.avRegistry.view<Position, PreviousPosition, AVEntityState,
                                    GraphicState, ClientGraphicState>()};
    for (auto [entity, position, previousPosition, avEntityState,
               graphicState, clientGraphicState] :
         view.each()) {
        // If the current phase has completed, increment to the next.
        if (!incrementPhaseIfNecessary(avEntityState, position, graphicState,
                                       clientGraphicState, currentTime)) {
            world.avRegistry.destroy(entity);
            continue;
        }

        // If all phases have completed, delete the entity.
        const AVEntity& avEntity{avEntityState.avEntity.get()};
        if (avEntityState.currentPhaseIndex == avEntity.phases.size()) {
            world.avRegistry.destroy(entity);
            continue;
        }

        // Entity is still alive, update it.
        if (!updateAVEntity(avEntityState, position, previousPosition,
                            graphicState, clientGraphicState)) {
            world.avRegistry.destroy(entity);
            continue;
        }
    }
}

bool AVSystem::incrementPhaseIfNecessary(AVEntityState& avEntityState,
                                         const Position& position,
                                         const GraphicState& graphicState,
                                         ClientGraphicState& clientGraphicState,
                                         double currentTime)
{
    // Determine the target's position.
    const AVEntity& avEntity{avEntityState.avEntity.get()};
    const AVEntity::Phase& lastPhase{
        avEntity.phases.at(avEntityState.currentPhaseIndex)};
    std::optional<Position> lastTargetOpt{
        AVEntityHelpers::getTargetPosition(
            lastPhase.behavior, avEntityState.targetEntity,
            static_cast<Position>(avEntityState.targetPosition), position,
            world.registry, false)};
    if (!lastTargetOpt) {
        return false;
    }
    Position lastTargetPosition{lastTargetOpt.value()};

    // If the entity has reached the completion condition for this phase, 
    // move to the next phase.
    const EntityGraphicSet& lastGraphicSet{
        graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
    GraphicRef lastGraphic{
        lastGraphicSet.graphics.at(clientGraphicState.graphicType)
            .at(clientGraphicState.graphicDirection)};
    if (AVEntityHelpers::timeElapsed(
            lastPhase.behavior, avEntityState.phaseStartTime,
            lastPhase.durationS, currentTime, lastGraphic)
        || AVEntityHelpers::positionReached(lastPhase.behavior, position,
                                            lastTargetPosition)) {
        avEntityState.currentPhaseIndex++;

        // If all phases have completed, exit early.
        if (avEntityState.currentPhaseIndex == avEntity.phases.size()) {
            return true;
        }

        avEntityState.setStartTime = true;

        // If the graphic changed, flag the graphic start time to be reset.
        const AVEntity::Phase& newPhase{
            avEntity.phases.at(avEntityState.currentPhaseIndex)};
        if (lastPhase.graphicSetID != newPhase.graphicSetID) {
            clientGraphicState.setStartTime = true;
        }
    }

    return true;
}

bool AVSystem::updateAVEntity(const AVEntityState& avEntityState,
                              Position& position,
                              PreviousPosition& previousPosition,
                              GraphicState& graphicState,
                              ClientGraphicState& clientGraphicState)
{
    // Determine the target's position.
    const AVEntity& avEntity{avEntityState.avEntity.get()};
    const AVEntity::Phase& phase{
        avEntity.phases.at(avEntityState.currentPhaseIndex)};
    std::optional<Position> targetOpt{AVEntityHelpers::getTargetPosition(
        phase.behavior, avEntityState.targetEntity,
        static_cast<Position>(avEntityState.targetPosition), position,
        world.registry, false)};
    if (!targetOpt) {
        return false;
    }
    Position targetPosition{targetOpt.value()};

    // Save the entity's old position.
    previousPosition = position;

    // Move the entity towards the target.
    position = position.moveTowards(targetPosition, phase.movementSpeed);

    // Get the entity's desired graphic state.
    auto graphicStateOpt{AVEntityHelpers::getGraphicState(
        phase.behavior, position, targetPosition)};
    if (!graphicStateOpt) {
        return false;
    }
    auto& [desiredGraphicType, desiredGraphicDirection]
        = graphicStateOpt.value();

    // Set the new graphic, accounting for missing graphics in the set.
    graphicState.graphicSetID = phase.graphicSetID;
    const EntityGraphicSet& graphicSet{
        graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
    GraphicHelpers::GraphicReturn newGraphic{
        GraphicHelpers::getGraphicOrFallback(
            graphicSet, desiredGraphicType, desiredGraphicDirection,
            desiredGraphicType, desiredGraphicDirection)};
    if (newGraphic.type != clientGraphicState.graphicType) {
        clientGraphicState.graphicType = newGraphic.type;
        clientGraphicState.setStartTime = true;
    }
    if (newGraphic.direction != clientGraphicState.graphicDirection) {
        clientGraphicState.graphicDirection = newGraphic.direction;
        // Note: We don't reset the animation start time, since we want e.g. 
        //       a run animation to play smoothly when changing direction.
    }

    return true;
}

} // End namespace Client
} // End namespace AM
