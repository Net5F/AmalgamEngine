#pragma once

#include "Name.h"
#include "AnimationState.h"
#include "Interaction.h"
#include "boost/mp11/list.hpp"

namespace AM
{

/**
 * All of the engine's component types that should be observed and auto-
 * replicated.
 *
 * See ComponentTypeRegistry.h for more info.
 *
 * Note: Every type in this list must also be in EngineReplicatedComponentTypes.
 * Note: For performance reasons, if something is going to be updated very
 *       frequently (e.g. movement), consider manually handling it in a system
 *       instead of adding it here.
 */
using EngineObservedComponentTypes
    = boost::mp11::mp_list<Name, AnimationState, Interaction>;

} // End namespace AM
