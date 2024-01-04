#pragma once

// Use the project's list, if one is provided.
#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
#include "Override/TypeLists/ProjectPersistedComponentTypes.h"
#else
#include "boost/mp11/list.hpp"

namespace AM
{
// Note: Persisted components are server-only.
namespace Server
{
/**
 * All of the project's component types that should be saved to the database 
 * and loaded at startup.
 */
using ProjectPersistedComponentTypes = boost::mp11::mp_list<>;

} // End namespace Server
} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
