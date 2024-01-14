#pragma once

#include "Name.h"
#include "Position.h"
#include "Rotation.h"
#include "AnimationState.h"
#include "Interaction.h"
#include "ItemHandlers.h"
#include "boost/mp11/list.hpp"

namespace AM
{
// Note: Persisted components are server-only.
namespace Server
{

/**
 * All of the engine's component types that should be saved to the database 
 * and loaded at startup.
 *
 * Note: Rotation implies Input and PreviousPosition (movement components).
 *       AnimationState implies Collision (graphics components).
 * Note: We currently don't have a versioning system for these. If you make a 
 *       change to either a component or the list itself, you must write a 
 *       program to perform the database migration.
 */
using EnginePersistedComponentTypes
    = boost::mp11::mp_list<Name, Position, Rotation, AnimationState,
                           Interaction, ItemHandlers>;

} // End namespace Server
} // End namespace AM
