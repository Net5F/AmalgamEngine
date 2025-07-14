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
        const GraphicDataBase&,
        std::function<void(CastableID, const Castable&)> addCastable)
    {
        addCastable(ItemInteractionType::Examine,
                    Castable{.triggersGCD{false}, .isTickSynchronized{false}});

        addCastable(
            EntityInteractionType::Talk,
            {.range{SharedConfig::CAST_ENTITY_INTERACTION_STANDARD_RANGE},
             .triggersGCD{false},
             .isTickSynchronized{false}});
    }
};

} // namespace AM
