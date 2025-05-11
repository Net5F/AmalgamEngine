#include "AVSystem.h"
#include "World.h"
#include "GraphicData.h"
#include "AVEffects.h"
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

} // End namespace Client
} // End namespace AM
