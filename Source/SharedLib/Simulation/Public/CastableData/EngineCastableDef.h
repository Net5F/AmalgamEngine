#pragma once

#include "CastableID.h"
#include "Castable.h"
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
        std::function<void(CastableID, const Castable&)> addCastable)
    {
        addCastable(ItemInteractionType::Examine,
                    Castable{.triggersGCD{false}});

        // TODO: Add a constant for standard interaction range
        addCastable(EntityInteractionType::Talk,
                    Castable{.range{32}, .triggersGCD{false}});
    }
};

} // namespace AM
