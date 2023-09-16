#pragma once

#include "EngineComponentLists.h"
#include "ProjectComponentLists.h"
#include "boost/mp11/list.hpp"
#include "boost/mp11/algorithm.hpp"
#include <variant>
#include "bitsery/bitsery.h"

namespace AM
{

/**
 * See comment in EngineComponentLists.h
 */
using ReplicatedComponentTypes
    = boost::mp11::mp_append<EngineComponentLists::ReplicatedComponentTypes,
                             ProjectComponentLists::ReplicatedComponentTypes>;
static_assert(boost::mp11::mp_size<ReplicatedComponentTypes>::value
                  <= (SDL_MAX_UINT8 + 1),
              "Too many types in ReplicatedComponentTypes. Max is 256");

/**
 * A variant that holds a client-relevant component.
 *
 * Used by the server to send components to the client (so it can replicate 
 * them).
 */
using ReplicatedComponent
    = boost::mp11::mp_rename<ReplicatedComponentTypes, std::variant>;

template<typename S>
void serialize(S& serializer, ReplicatedComponent& replicatedComponent)
{
    std::visit([&](auto& component) { serializer.object(component); },
               replicatedComponent);
}

} // End namespace AM
