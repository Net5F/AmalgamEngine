#pragma once

#include "boost/mp11/list.hpp"

// Use the project's ProjectComponentLists, if one is provided.
//#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
//#include "Override/ProjectComponentLists.h"
//#else
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
 * add it to ObservedComponentMap below.
 */
using ReplicatedComponentTypes = boost::mp11::mp_list<>;

/**
 * A map of "ObservedComponent" -> "SendComponents".
 * The first type in each list is the key, the rest are the values.
 *
 * When an ObservedComponent is updated (using e.g. patch()), an Update message
 * will be sent by the server to all nearby clients, containing the 
 * SendComponents.
 *
 * Note: All of the SendComponents must be in ReplicatedComponentTypes.
 *       The ObservedComponents don't need to be in ReplicatedComponentTypes.
 */
using ObservedComponentMap = boost::mp11::mp_list<>;

} // End namespace ProjectComponentLists
} // End namespace AM

//#endif // defined(AM_OVERRIDE_CONFIG)
