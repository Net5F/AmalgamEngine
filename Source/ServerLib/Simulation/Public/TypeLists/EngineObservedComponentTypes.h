#pragma once

#include "Name.h"
#include "MovementModifiers.h"
#include "GraphicState.h"
#include "CollisionBitSets.h"
#include "Interaction.h"
#include "boost/mp11/list.hpp"

namespace AM
{
// Note: Observed components are server-only.
namespace Server
{
/**
 * See EngineReplicatedComponentTypes.h for more info.
 */
using EngineObservedComponentTypes
    = boost::mp11::mp_list<Name, MovementModifiers, GraphicState,
                           CollisionBitSets, Interaction>;

} // End namespace Server
} // End namespace AM
