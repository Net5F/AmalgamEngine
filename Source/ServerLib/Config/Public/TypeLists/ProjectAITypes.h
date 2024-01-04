#pragma once

// Use the project's list, if one is provided.
#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
#include "Override/TypeLists/ProjectAITypes.h"
#else
#include "boost/mp11/list.hpp"

namespace AM
{
// Note: AI is server-only.
namespace Server
{
/**
 * Add AI classes to this list to have them be processed by the engine.
 * 
 * Note: Every type in this list must be derived from AILogic.
 */
using ProjectAITypes = boost::mp11::mp_list<>;

} // End namespace Server
} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
