#pragma once

#include "EngineReplicatedComponentTypes.h"
#include "ProjectReplicatedComponentTypes.h"
#include "boost/mp11/list.hpp"
#include "boost/mp11/algorithm.hpp"
#include <variant>

namespace AM
{

/**
 * See comment in EngineReplicatedComponentTypes.h
 */
using ReplicatedComponentTypes
    = boost::mp11::mp_append<EngineReplicatedComponentTypes,
                             ProjectReplicatedComponentTypes>;
// Note: This limit is only to prevent data sizes from getting larger. If we 
//       let there be more than 256 components, every component's index would 
//       start taking 2B instead of 1B.
static_assert(boost::mp11::mp_size<ReplicatedComponentTypes>::value
                  <= (SDL_MAX_UINT8 + 1),
              "Too many types in ReplicatedComponentTypes. Max is 256.");

/**
 * A variant that holds a client-relevant component.
 *
 * Used by the server to send components to the client (so it can replicate
 * them).
 */
using ReplicatedComponent
    = boost::mp11::mp_rename<ReplicatedComponentTypes, std::variant>;

} // End namespace AM
