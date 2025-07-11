#pragma once

#include "EntityInteractionType.h"
#include "ItemInteractionType.h"
#include "EntityGraphicType.h"
#include "Log.h"
#include <SDL_stdinc.h>
#include <string>

namespace AM
{
/**
 * Shared functions for converting enums to display strings.
 */
namespace DisplayStrings
{
inline std::string get(EntityInteractionType interactionType)
{
    switch (interactionType) {
        // Engine interaction types.
        case EntityInteractionType::Talk:
            return "Talk";
        // Project interaction types.
        case EntityInteractionType::Replant:
            return "Replant";
        default:
            break;
    }

    LOG_ERROR("Tried to get display string for unknown interaction type.");
    return "?";
}

inline std::string get(ItemInteractionType interactionType)
{
    switch (interactionType) {
        // Engine interaction types.
        case ItemInteractionType::UseOn:
            return "Use";
        case ItemInteractionType::Destroy:
            return "Destroy";
        case ItemInteractionType::Examine:
            return "Examine";
        // Project interaction types.
        default:
            break;
    }

    LOG_ERROR("Tried to get display string for unknown interaction type.");
    return "?";
}

inline std::string get(EntityGraphicType graphicType)
{
    switch (graphicType) {
        // Engine graphic types.
        case EntityGraphicType::Idle:
            return "Idle";
        case EntityGraphicType::Run:
            return "Run";
        case EntityGraphicType::Crouch:
            return "Crouch";
        case EntityGraphicType::Jump:
            return "Jump";
        // Project graphic types.
        default:
            break;
    }

    LOG_ERROR("Tried to get display string for unknown graphic type.");
    return "?";
}

} // namespace DisplayStrings
} // namespace AM
