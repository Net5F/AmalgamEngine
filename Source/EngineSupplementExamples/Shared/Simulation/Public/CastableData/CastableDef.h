#pragma once

#include "CastableID.h"
#include "Castable.h"
#include "GraphicDataBase.h"
#include <functional>

namespace AM
{

/**
 * Use this class to define all of the castables in your project (item 
 * interactions, entity interactions, and spells).
 */
class CastableDef {
public:
    static void defineCastables(const GraphicDataBase& graphicData,
        std::function<void(CastableID, const Castable&)> addCastable)
    {
        // Fireball
        {
            Castable castable{
                .range{64},
                .castTime{1},
                .cooldownTime{2},
                .triggersGCD{true},
                .castingGraphicType{EntityGraphicType::Crouch},
                //.castCompleteGraphicType{EntityGraphicType::Crouch}
            };

            //VisualEffect visualEffect{
            //    .graphicID{toGraphicID(
            //        graphicData.getAnimation("explosion").numericID)},
            //    .loopMode{VisualEffect::LoopMode::Loop},
            //    .loopTime{3}};
            //castable.castCompleteVisualEffects.emplace_back(visualEffect);

            AVEntity avEntity{.startDistance{10}, .canMoveVertically{true}};
            AVEntity::Phase phase{
                .graphicSetID{
                    graphicData.getEntityGraphicSet("Fireball").numericID},
                .behavior{AVEntity::Behavior::MoveToEntity},
                .movementSpeed{5.f}};
            avEntity.phases.emplace_back(phase);
            castable.castCompleteAVEntities.emplace_back(avEntity);

            addCastable(SpellType::Fireball, castable);
        }
    }
};

} // namespace AM
