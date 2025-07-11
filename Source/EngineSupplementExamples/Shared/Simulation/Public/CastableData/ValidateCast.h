#pragma once

#include "Castable.h"
#include "Castable.h"
#include "entt/fwd.hpp"
#include "entt/entity/registry.hpp"
#include <vector>

namespace AM
{

/**
 * This function is called when an entity is trying to cast something. Use it 
 * to check if the entity meets the Castable's requirements.
 *
 * @return true if casterEntity is allowed to cast castable. Else, false.
 */
inline bool validateCast(entt::entity casterEntity, const Castable& castable,
                         const entt::registry& registry)
{
    // const Stats* stats{registry.try_get(casterEntity)};
    // if (!stats) {
    //     // Can't cast without stats. Return early.
    //     return false;
    // }

    // If casterEntity doesn't meet one of the requirements, return false.
    //for (const Castable::Requirement& requirement : castable.requirements) {
    //    switch (requirement.type) {
    //        case CastableRequirementType::StatStrength:
    //            if (stats.strength < requirementValue) {
    //                return false;
    //            }
    //            break;
    //        default:
    //            break;
    //    }
    //}

    return true;
}

} // End namespace AM
