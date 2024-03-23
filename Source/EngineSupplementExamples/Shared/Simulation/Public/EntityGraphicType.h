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
 * Controls the graphic types that show up for entity graphic sets in  
 * ResourceImporter.
 */
enum class EntityGraphicType : Uint8 {
    // Engine graphic types (copied here so we can use one strongly-typed enum).
    NotSet = static_cast<Uint8>(EngineEntityGraphicType::NotSet),
    IdleSouth = static_cast<Uint8>(EngineEntityGraphicType::IdleSouth),
    IdleSouthWest = static_cast<Uint8>(EngineEntityGraphicType::IdleSouthWest),
    IdleWest = static_cast<Uint8>(EngineEntityGraphicType::IdleWest),
    IdleNorthWest = static_cast<Uint8>(EngineEntityGraphicType::IdleNorthWest),
    IdleNorth = static_cast<Uint8>(EngineEntityGraphicType::IdleNorth),
    IdleNorthEast = static_cast<Uint8>(EngineEntityGraphicType::IdleNorthEast),
    IdleEast = static_cast<Uint8>(EngineEntityGraphicType::IdleEast),
    IdleSouthEast = static_cast<Uint8>(EngineEntityGraphicType::IdleSouthEast),
    RunSouth = static_cast<Uint8>(EngineEntityGraphicType::RunSouth),
    RunSouthWest = static_cast<Uint8>(EngineEntityGraphicType::RunSouthWest),
    RunWest = static_cast<Uint8>(EngineEntityGraphicType::RunWest),
    RunNorthWest = static_cast<Uint8>(EngineEntityGraphicType::RunNorthWest),
    RunNorth = static_cast<Uint8>(EngineEntityGraphicType::RunNorth),
    RunNorthEast = static_cast<Uint8>(EngineEntityGraphicType::RunNorthEast),
    RunEast = static_cast<Uint8>(EngineEntityGraphicType::RunEast),
    RunSouthEast = static_cast<Uint8>(EngineEntityGraphicType::RunSouthEast),
    // TODO: Crouch, Jump

    // Project graphic types.
    // MyGraphicType =
    // static_cast<Uint8>(EngineEntityGraphicType::PROJECT_START),
};

/** The number of entity graphic types (doesn't include NotSet).
    Must be maintained if EntityGraphicType is changed. */
static constexpr Uint8 ENTITY_GRAPHIC_TYPE_COUNT{16};

} // End namespace AM
