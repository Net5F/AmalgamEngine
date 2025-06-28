#pragma once

#include "EngineEntityGraphicType.h"
#include "AMAssert.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of graphics that may be displayed to represent an entity, 
 * depending on what that entity's state is.
 *
 * Each entity graphic type can face 8 directions, depending on the entity's 
 * Rotation component.
 *
 * Controls the graphic types that show up for entity graphic sets in  
 * ResourceImporter.
 */
enum class EntityGraphicType : Uint8 {
    // Engine graphic types (copied here so we can use one strongly-typed enum).
    NotSet = static_cast<Uint8>(EngineEntityGraphicType::NotSet),
    /** Note: All graphic sets are expected to contain IdleSouth. */
    Idle = static_cast<Uint8>(EngineEntityGraphicType::Idle),
    Run = static_cast<Uint8>(EngineEntityGraphicType::Run),
    Crouch = static_cast<Uint8>(EngineEntityGraphicType::Crouch),
    Jump = static_cast<Uint8>(EngineEntityGraphicType::Jump),

    // Project graphic types.
    PROJECT_START = static_cast<Uint8>(EngineEntityGraphicType::PROJECT_START),
    // MyGraphicType,

    PROJECT_END 
};

} // End namespace AM
