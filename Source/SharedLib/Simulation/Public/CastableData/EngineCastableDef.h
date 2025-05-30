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

        addCastable(
            EntityInteractionType::Talk,
            Castable{
                .range{SharedConfig::CAST_ENTITY_INTERACTION_STANDARD_RANGE},
                .castTime{2},
                .cooldownTime{2},
                .triggersGCD{false},
                .castingGraphicType{EntityGraphicType::Run},
                //.castCompleteGraphicType{EntityGraphicType::Crouch},
                .castCompleteVisualEffects{
                    {toGraphicID(
                         graphicData.getAnimation("explosion").numericID),
                     VisualEffect::LoopMode::Loop, 3}}});
    }
};

} // namespace AM
