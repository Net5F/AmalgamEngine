#pragma once

// Use the project's list, if one is provided.
#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
#include "Override/TypeLists/ProjectObservedComponentTypes.h"
#else
#include "boost/mp11/list.hpp"

namespace AM
{
/**
 * All of the engine's component types that should be observed and auto-
 * replicated. When an observed component is updated (using patch() or
 * replace()), an Update message containing the component will be sent by the
 * server to all nearby clients.
 *
 * Note: Every type in this list must also be in ProjectReplicatedComponentTypes.
 * Note: For performance reasons, if something is going to be updated very
 *       frequently (e.g. movement), consider manually handling it in a system
 *       instead of adding it here.
 */
using ProjectObservedComponentTypes = boost::mp11::mp_list<>;

} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
