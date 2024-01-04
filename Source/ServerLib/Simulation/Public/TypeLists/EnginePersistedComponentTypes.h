#pragma once

#include "Name.h"
#include "Position.h"
#include "AnimationState.h"
#include "Interaction.h"
#include "boost/mp11/list.hpp"

namespace AM
{

/**
 * All of the engine's component types that should be saved to the database 
 * and loaded at startup.
 */
using EnginePersistedComponentTypes
    = boost::mp11::mp_list<Name, Position, AnimationState, Interaction,
                           ItemHandlers>;

} // End namespace AM
