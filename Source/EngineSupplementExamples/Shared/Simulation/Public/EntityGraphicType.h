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
    CrouchSouth = static_cast<Uint8>(EngineEntityGraphicType::CrouchSouth),
    CrouchSouthWest = static_cast<Uint8>(EngineEntityGraphicType::CrouchSouthWest),
    CrouchWest = static_cast<Uint8>(EngineEntityGraphicType::CrouchWest),
    CrouchNorthWest = static_cast<Uint8>(EngineEntityGraphicType::CrouchNorthWest),
    CrouchNorth = static_cast<Uint8>(EngineEntityGraphicType::CrouchNorth),
    CrouchNorthEast = static_cast<Uint8>(EngineEntityGraphicType::CrouchNorthEast),
    CrouchEast = static_cast<Uint8>(EngineEntityGraphicType::CrouchEast),
    CrouchSouthEast = static_cast<Uint8>(EngineEntityGraphicType::CrouchSouthEast),
    JumpSouth = static_cast<Uint8>(EngineEntityGraphicType::JumpSouth),
    JumpSouthWest = static_cast<Uint8>(EngineEntityGraphicType::JumpSouthWest),
    JumpWest = static_cast<Uint8>(EngineEntityGraphicType::JumpWest),
    JumpNorthWest = static_cast<Uint8>(EngineEntityGraphicType::JumpNorthWest),
    JumpNorth = static_cast<Uint8>(EngineEntityGraphicType::JumpNorth),
    JumpNorthEast = static_cast<Uint8>(EngineEntityGraphicType::JumpNorthEast),
    JumpEast = static_cast<Uint8>(EngineEntityGraphicType::JumpEast),
    JumpSouthEast = static_cast<Uint8>(EngineEntityGraphicType::JumpSouthEast),

    // Project graphic types.
    // MyGraphicType =
    // static_cast<Uint8>(EngineEntityGraphicType::PROJECT_START),
};

/** The number of entity graphic types (doesn't include NotSet).
    Must be maintained if EntityGraphicType is changed. */
static constexpr Uint8 ENTITY_GRAPHIC_TYPE_COUNT{32};

/**
 * Converts an entity graphic type into an array index.
 *
 * The assumed array spans all valid graphic types, and matches the order 
 * of their values in the enum.
 */
constexpr std::size_t toIndex(EntityGraphicType graphicType)
{
    AM_ASSERT(graphicType != EntityGraphicType::NotSet,
              "Tried to get index of uninitialized entity graphic type.");

    return static_cast<std::size_t>(graphicType) - 1;
}

/**
 * Converts an array index into an entity graphic type.
 *
 * The assumed array spans all valid graphic types, and matches the order 
 * of their values in the enum.
 */
constexpr EntityGraphicType toEntityGraphicType(Uint8 index)
{
    AM_ASSERT(index < ENTITY_GRAPHIC_TYPE_COUNT,
              "Tried to get entity graphic type with too-large index.");
    return static_cast<EntityGraphicType>(index + 1);
}

} // End namespace AM
