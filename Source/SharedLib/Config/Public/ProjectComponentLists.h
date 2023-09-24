#pragma once

#include "boost/mp11/list.hpp"

// Use the project's ProjectComponentLists, if one is provided.
#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
#include "Override/ProjectComponentLists.h"
#else
namespace AM
{
namespace ProjectComponentLists
{

/**
 * All of the project's component types that are relevant to the client.
 *
 * When a client comes in range of an entity, an Init message that includes these
 * components will be sent (if the entity possesses any of them).
 *
 * In other words, adding components to this list will cause them to be sent 
 * once. If you want a component to additionally be sent whenever it's updated, 
 * add it to ObservedComponentTypes below.
 */
using ReplicatedComponentTypes = boost::mp11::mp_list<>;

/**
 * All of the engine's component types that should be observed and auto-
 * replicated. When an observed component is updated (using patch() or 
 * replace()), an Update message containing the component will be sent by the 
 * server to all nearby clients.
 *
 * Note: Every type in this list must also be in ReplicatedComponentTypes.
 * Note: For performance reasons, if something is going to be updated very 
 *       frequently (e.g. movement), consider manually handling it in a system 
 *       instead of adding it here.
 */
using ObservedComponentTypes = boost::mp11::mp_list<>;

} // End namespace ProjectComponentLists
} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
