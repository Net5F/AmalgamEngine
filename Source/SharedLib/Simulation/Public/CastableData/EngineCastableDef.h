#pragma once

#include "CastableID.h"
#include "Castable.h"
#include "GraphicDataBase.h"
#include "SharedConfig.h"
#include <functional>

namespace AM
{

/**
 * Defines all of the engine-provided castables (item interactions, entity 
 * interactions, and spells).
 */
class EngineCastableDef {
public:
    static void defineCastables(
        const GraphicDataBase& graphicData,
        std::function<void(CastableID, const Castable&)> addCastable)
    {
        addCastable(ItemInteractionType::Examine,
                    Castable{.triggersGCD{false}});

        // Talk
        {
            Castable castable{
                //.range{SharedConfig::CAST_ENTITY_INTERACTION_STANDARD_RANGE},
                .range{128},
                .castTime{1},
                .cooldownTime{2},
                .triggersGCD{false},
                .castingGraphicType{EntityGraphicType::Crouch},
                //.castCompleteGraphicType{EntityGraphicType::Crouch}
            };

            //VisualEffect visualEffect{
            //    .graphicID{toGraphicID(
            //        graphicData.getAnimation("explosion").numericID)},
            //    .loopMode{VisualEffect::LoopMode::Loop},
            //    .loopTime{3}};
            //castable.castCompleteVisualEffects.emplace_back(visualEffect);

            AVEntity avEntity{.startDistance{4}, .canMoveVertically{true}};
            AVEntity::Phase phase{
                .graphicSetID{
                    graphicData.getEntityGraphicSet("Fireball").numericID},
                .behavior{AVEntity::Behavior::MoveToEntity},
                .movementSpeed{4.f}};
            avEntity.phases.emplace_back(phase);
            castable.castCompleteAVEntities.emplace_back(avEntity);

            addCastable(EntityInteractionType::Talk, castable);
        }
    }
};

} // namespace AM
